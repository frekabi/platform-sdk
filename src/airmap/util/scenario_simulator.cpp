#include <airmap/util/scenario_simulator.h>

#include <airmap/codec.h>
#include <airmap/telemetry.h>

#include <chrono>

namespace {
constexpr const char* component{"ScenarioSimulator"};
}  // namespace

airmap::util::ScenarioSimulator::Runner::Runner(std::uint32_t frequency) : frequency_{frequency}, running_{false} {
}

airmap::util::ScenarioSimulator::Runner::~Runner() {
  stop();
  if (worker_.joinable())
    worker_.join();
}

void airmap::util::ScenarioSimulator::Runner::start(const std::shared_ptr<ScenarioSimulator>& simulator,
                                                    const std::shared_ptr<Client>& client) {
  std::cout << simulator << std::endl;
  if (!running_.exchange(true)) {
    worker_ = std::thread{[this, simulator, client]() {
      simulator->start();
      while (running_.load()) {
        simulator->update(
            [this, client](const DateTime& now, const Scenario::Participant& p, const Geometry::Coordinate& c) {
              if (p.flight && p.encryption_key) {
                Telemetry::Position position;
                position.timestamp           = milliseconds_since_epoch(now);
                position.latitude            = c.latitude;
                position.longitude           = c.longitude;
                position.altitude_msl        = 100.;
                position.altitude_gl         = 100.;
                position.horizontal_accuracy = 2.;

                Telemetry::Update update{position};
                client->telemetry().submit_updates(p.flight.get(), p.encryption_key.get(), {update});
              }
            });
        std::this_thread::sleep_for(std::chrono::milliseconds{1000 / frequency_});
      }
    }};
  }
}

void airmap::util::ScenarioSimulator::Runner::stop() {
  running_.exchange(false);
}

airmap::util::ScenarioSimulator::ScenarioSimulator(const Scenario& scenario, const std::shared_ptr<Logger>& logger)
    : scenario_{scenario}, log_{logger} {
  for (const auto& participant : scenario_.participants) {
    state_.emplace_back(participant.geometry.details_for_polygon());
  }
}

void airmap::util::ScenarioSimulator::start() {
  last_update_ = Clock::universal_time();
}

void airmap::util::ScenarioSimulator::update(
    const std::function<void(const DateTime&, const Scenario::Participant&, const Geometry::Coordinate&)>& enumerator) {
  auto now = Clock::universal_time();

  for (std::size_t i = 0; i < scenario_.participants.size(); i++) {
    const auto& participant = scenario_.participants.at(i);
    auto new_state          = state_.at(i).update(now);
    log_.infof(component, "updating participant %s on %s: (%f, %f)", participant.pilot.user_name,
               participant.aircraft.id, new_state.latitude, new_state.longitude);

    enumerator(now, participant, new_state);
  }

  last_update_ = now;
}

void airmap::util::from_json(const nlohmann::json& j, Scenario& s) {
  codec::json::get(s.name, j, "name");
  codec::json::get(s.description, j, "description");
  codec::json::get(s.participants, j, "participants");
}

void airmap::util::from_json(const nlohmann::json& j, Scenario::Participant& p) {
  codec::json::get(p.aircraft, j, "aircraft");
  codec::json::get(p.pilot, j, "pilot");
  codec::json::get(p.geometry, j, "geometry");
  codec::json::get(p.velocity, j, "velocity");
}

void airmap::util::from_json(const nlohmann::json& j, std::vector<Scenario::Participant>& p) {
  for (const auto& element : j) {
    p.push_back(Scenario::Participant{});
    p.back() = element;
  }
}