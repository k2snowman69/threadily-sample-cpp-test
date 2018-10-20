// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
#include <thread>
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
    describe("ReadyEvent", []() {

        it("ReadyEvent_FinishedThenWait", [&]() {
            ReadyEvent re;

            re.finished();
            re.wait();

            // if the test didn't freeze, we are successful
        });

        it("ReadyEvent_WaitThenFinished", [&]() {
            auto threadManager = std::make_shared<ThreadManager>();
            auto uiThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);
            auto appThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_App);

            ReadyEvent re;
            bool isFinished = false;

            uiThread->addWork(std::make_shared<ThreadQueueItem>([&re, &isFinished]() {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                isFinished = true;
                re.finished();
            }));
            re.wait();

            AssertThat(isFinished, Equals(true));
        });

        it("TestName", [&]() {
        });
    });
});
}
}