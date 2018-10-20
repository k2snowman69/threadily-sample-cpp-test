// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ThreadIds.h>
#include <ThreadManager.h>

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
	describe("ThreadFactoryUnitTest", []() {
		it("ThreadManager_Create_Success", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			// create the thread
			auto thread = threadManager->getOrCreateThread(0);
			AssertThat(thread.use_count(), !Equals(0));
		});

		// lower thread id's only notify the thread +1 from it unless it is specified not to do otherwise
		it("ThreadManager_Create_Success_Notification", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			// create threads
			auto thread0 = threadManager->getOrCreateThread(0, std::set<unsigned int>({1}));
			auto thread5 = threadManager->getOrCreateThread(5, std::set<unsigned int>({0}));
			auto thread2 = threadManager->getOrCreateThread(2, std::set<unsigned int>({3}));
			auto thread3 = threadManager->getOrCreateThread(3, std::set<unsigned int>({2}));
			auto thread1 = threadManager->getOrCreateThread(1, std::set<unsigned int>({2}));

			AssertThat(thread0->isNotifiedBy(thread5), IsTrue());
			AssertThat(thread0->isNotifiedBy(thread1), IsFalse());
			AssertThat(thread0->isNotifiedBy(thread2), IsFalse());
			AssertThat(thread0->isNotifiedBy(thread3), IsFalse());

			AssertThat(thread2->isNotifiedBy(thread0), IsFalse());
			AssertThat(thread2->isNotifiedBy(thread3), IsTrue());
			AssertThat(thread2->isNotifiedBy(thread5), IsFalse());

			AssertThat(thread5->isNotifiedBy(thread0), IsFalse());
			AssertThat(thread5->isNotifiedBy(thread2), IsFalse());
			AssertThat(thread5->isNotifiedBy(thread3), IsFalse());
		});

		it("ThreadManager_Create_Success_Notification_2", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			auto uiThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			auto appThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_Service, ThreadIds::ThreadId_UI}), nullptr);
			auto serviceThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);

			AssertThat(uiThread->isNotifiedBy(appThread), IsTrue());
			AssertThat(uiThread->isNotifiedBy(serviceThread), IsFalse());

			AssertThat(appThread->isNotifiedBy(serviceThread), IsTrue());
			AssertThat(appThread->isNotifiedBy(uiThread), IsTrue());

			AssertThat(serviceThread->isNotifiedBy(uiThread), IsFalse());
			AssertThat(serviceThread->isNotifiedBy(appThread), IsTrue());
		});

		it("ThreadManager_Lifecycle_Success", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			// create the thread
			auto thread = threadManager->getOrCreateThread(0);
			AssertThat(thread.use_count(), !Equals(0));

			// now we check to make sure the thread stays alive even after we destroy our local copy of it
			auto beforeThreadNull = thread.get();
			thread = nullptr;
			thread = threadManager->getOrCreateThread(0);
			AssertThat(beforeThreadNull, Equals(thread.get()));

			threadManager->clear();
		});
	});
});
}
}