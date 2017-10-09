#ifndef AIRMAP_CMDS_AIRMAP_CMD_TEST_LAANC_PHOENIX_H_
#define AIRMAP_CMDS_AIRMAP_CMD_TEST_LAANC_PHOENIX_H_

#include <airmap/cmds/airmap/cmd/test.h>

#include <airmap/flight_plans.h>
#include <airmap/pilots.h>

namespace airmap {
namespace cmds {
namespace airmap {
namespace cmd {
namespace test {
namespace laanc {

class Phoenix : public Test::Suite {
 public:
  static constexpr const char* name{"laanc.phoenix"};

  void run(const std::shared_ptr<Logger>& logger, const std::shared_ptr<::airmap::Client>& client,
           const std::shared_ptr<::airmap::Context>& context, const ::airmap::Token& token) override;

 private:
  FlightPlans::Create::Parameters parameters_for_zoo();

  void query_pilot();
  void handle_query_pilot_finished(const Pilots::Authenticated::Result& result);

  void query_aircrafts();
  void handle_query_aircrafts_finished(const Pilots::Aircrafts::Result& result);

  void plan_flight();
  void handle_plan_flight_finished(const FlightPlans::Create::Result& result);

  void render_briefing(const FlightPlan::Id& id);
  void handle_render_briefing_finished(const FlightPlans::RenderBriefing::Result& result, const FlightPlan::Id& id);

  void submit_flight_plan(const FlightPlan::Id& id);
  void handle_submit_flight_plan_finished(const FlightPlans::Submit::Result& result);

  util::FormattingLogger log_{create_null_logger()};
  std::shared_ptr<::airmap::Client> client_;
  std::shared_ptr<::airmap::Context> context_;
  ::airmap::Token token_;
  Optional<Pilot> pilot_;
  Optional<Pilot::Aircraft> aircraft_;
};

}  // namespace laanc
}  // namespace test
}  // namespace cmd
}  // namespace airmap
}  // namespace cmds
}  // namespace airmap

#endif  // AIRMAP_CMDS_AIRMAP_CMD_TEST_LAANC_PHOENIX_H_