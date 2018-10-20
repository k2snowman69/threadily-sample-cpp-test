// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ReadyEvent.h>
#include <ThreadManager.h>
#include <ThreadQueueItem.h>
#include <ThreadIds.h>

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
    describe("TestGroup", []() {
        it("TestName", [&]() {
            AssertThat(true, Equals(true));
        });
    });
});
}
}