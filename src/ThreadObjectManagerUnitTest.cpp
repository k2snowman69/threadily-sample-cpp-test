// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ThreadIds.h>

#include <ThreadManager.h>
#include <ThreadObjectManager.h>

#include "CountingThreadObjectManager.h"
#include "PrimativesThreadObject.h"

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
	describe("ThreadObjectManagerUnitTest", []() {
		// Creates and destroys the same object a few times
		it("ThreadableObjectManager_CreateDestroyLoop", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			auto manager = std::make_shared<ThreadObjectManager<ThreadObject<>>>(threadManager);

			auto objectUI1 = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			AssertThat(objectUI1.get(), !IsNull());
			std::weak_ptr<ThreadObject<>> objectUI1Weak = objectUI1;
			objectUI1 = nullptr;

			AssertThat(objectUI1Weak.expired(), IsTrue());

			auto objectUI2 = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			AssertThat(objectUI2.get(), !IsNull());
			objectUI2 = nullptr;

			auto objectUI3 = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			AssertThat(objectUI3.get(), !IsNull());
		});

		// Creates and destroys the same object a few times between two threads App <-> UI
		it("ThreadableObjectManager_CreateDestroyLoop_LinkedObjects", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<ThreadObject<>>>(threadManager);

			auto objectUI1 = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			AssertThat(objectUI1.get(), !IsNull());
			std::weak_ptr<ThreadObject<>> objectUI1Weak = objectUI1;
			objectUI1 = nullptr;

			// Since it's a two way object, they hold each other alive
			AssertThat(objectUI1Weak.expired(), IsFalse());
		});

		// tests to make sure two thread objects get linked together
		// App <-> UI
		it("ThreadableObjectManager_ThreadObjectsLinking_1", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			AssertThat((size_t)1, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			// This is still 1 because the UI object holds onto a reference to the App object because it notifies it
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));
		});

		// tests to make sure when the linking looks like this:
		// Service <-> App <-> UI
		// and we create object 1 and 3 first... when we create object 2 everything should hook up correctly
		it("ThreadableObjectManager_ThreadObjectsLinking_2", [&]() {
			// create all three threads
			auto threadManager = std::make_shared<ThreadManager>();
			auto uiThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			auto appThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_Service, ThreadIds::ThreadId_UI}), nullptr);
			auto serviceThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			// create the ui first then the service thread. These should have nothing to do with one another
			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto objectService = manager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			AssertThat((size_t)1, Equals(objectService->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));

			// now create the app thread. This should be the connecting part to the ui and service threads
			auto objectApp = manager->getOrCreateObject(appThread->getThreadId(), 0);

			AssertThat((size_t)1, Equals(objectService->name->getSubscribersCount()));
			AssertThat((size_t)2, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			AssertThat((size_t)1, Equals(objectService->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));
		});

		// tests to make sure one way linking works
		// App -> UI
		it("ThreadableObjectManager_ThreadObjectsLinking_OneWay_UIFirst", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>(), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			AssertThat((size_t)1, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)0, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			AssertThat((size_t)0, Equals(objectUI->name->getSubscribersCount()));
		});

		// tests to make sure one way linking works
		// App -> UI
		it("ThreadableObjectManager_ThreadObjectsLinking_OneWay_AppFirst", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>(), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);
			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);

			AssertThat((size_t)1, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)0, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			AssertThat((size_t)0, Equals(objectUI->name->getSubscribersCount()));
		});

		// tests to make sure one way linking works
		// App <- UI
		it("ThreadableObjectManager_ThreadObjectsLinking_OneWayDown_UIFirst", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>(), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			AssertThat((size_t)0, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));
		});

		// tests to make sure one way linking works
		// App <- UI
		it("ThreadableObjectManager_ThreadObjectsLinking_OneWayDown_AppFirst", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>({ThreadIds::ThreadId_App}), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>(), nullptr);

			auto manager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);

			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);
			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);

			AssertThat((size_t)0, Equals(objectApp->name->getSubscribersCount()));
			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));

			objectApp = nullptr;

			AssertThat((size_t)1, Equals(objectUI->name->getSubscribersCount()));
		});

		it("ThreadableObjectManager_Subclassing_1", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI, std::set<unsigned int>(), nullptr);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}), nullptr);

			auto manager = std::make_shared<CountingThreadObjectManager<PrimativesThreadObject>>(threadManager);
			auto objectUI = manager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto objectApp = manager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			AssertThat(2U, Equals(manager->getCountOfObjectsCreated()));
		});
	});
});
}
}