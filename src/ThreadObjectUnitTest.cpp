// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ThreadIds.h>
#include <ThreadManager.h>
#include <ThreadObjectManager.h>
#include <ThreadObject.h>
#include <PrimativesThreadObject.h>
#include <ReadyEvent.h>
#include <ThreadablesThreadObject.h>

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
	describe("ThreadObjectUnitTest", []() {
		it("ThreadObject_Create_NullManager", [&]() {
			AssertThrows(std::runtime_error, std::make_shared<ThreadObject<>>(nullptr, ThreadIds::ThreadId_UI, 0));
		});
		it("ThreadObject_GetPeer_Success", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<ThreadObject<>>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 4);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 4);
		});
		// sets up the threads like so:
		// Service -> App -> UI
		// then posts a property change to the service thread
		// we expect this to go all the way to the UI thread even though the App object doesn't exist
		// If this test spins forever, then our test failed
		it("ThreadObject_NotifiesThroughMiddleLayer", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			ReadyEvent e;

			auto subscribeHandle = threadObject_UI->name->subscribe([&e](std::wstring newValue) {
				AssertThat(std::wstring(L"hi!"), Equals(newValue));
				e.finished();
			});

			AssertThat(1, Equals(threadObject_UI->name->getSubscribersCount()));
			AssertThat(1, Equals(threadObject_Service->name->getSubscribersCount()));

			threadObject_Service->name->set(L"hi!");
			e.wait();

			threadObject_UI->name->unsubscribe(subscribeHandle);
		});
		// sets up the threads like so:
		// Service -> App -> UI
		// then posts a property change to the service thread
		// we expect this to go all the way to the UI thread even though the App object doesn't exist
		// If this test spins forever, then our test failed
		it("ThreadObject_Observable_Vector_Primatives", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			// set up a waiter until we get a notification on the UI thread that something has been completed
			ReadyEvent e;
			auto subscribeHandle = threadObject_UI->intArray->subscribe([&e](int newValue, size_t index, ObservableActionType action) {
				e.finished();
			});

			threadObject_Service->intArray->set(0, 17);
			e.wait();

			AssertThat((size_t)1, Equals(threadObject_Service->intArray->size()));
			AssertThat((size_t)1, Equals(threadObject_UI->intArray->size()));

			AssertThat(17, Equals(threadObject_Service->intArray->at(0)));
			AssertThat(17, Equals(threadObject_UI->intArray->at(0)));

			threadObject_UI->intArray->unsubscribe(subscribeHandle);
		});
		// sets up the threads like so:
		// Service -> App -> UI
		// then posts a property change to an object within the service thread object
		it("ThreadObject_Observable_Object_ThreadObject", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<ThreadablesThreadObject>>(threadManager);
			auto childThreadObjectManager = std::make_shared<ThreadObjectManager<EmptyThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			// set up a waiter until we get a notification on the UI thread that something has been completed
			ReadyEvent e;
			auto subscribeHandle = threadObject_UI->emptyObject->subscribe([&e](std::shared_ptr<EmptyThreadObject> newValue) {
				e.finished();
			});

			auto newChildObject = childThreadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 4);
			threadObject_Service->emptyObject->set(newChildObject);
			e.wait();

			AssertThat(threadObject_UI->emptyObject->get(), !IsNull());
			AssertThat(ThreadIds::ThreadId_UI, Equals(threadObject_UI->emptyObject->get()->getThreadId()));
			AssertThat(ThreadIds::ThreadId_Service, Equals(threadObject_Service->emptyObject->get()->getThreadId()));
			AssertThat(threadObject_UI->emptyObject->get()->getId().instanceId, Equals(threadObject_Service->emptyObject->get()->getId().instanceId));
			AssertThat(threadObject_UI->emptyObject->get().get(), !Equals(threadObject_Service->emptyObject->get().get()));
			AssertThat(threadObject_UI->emptyObject->get()->getId().instanceId, Equals(threadObject_Service->emptyObject->get()->getId().instanceId));

			threadObject_UI->emptyObject->unsubscribe(subscribeHandle);
		});
		// sets up the threads like so:
		// Service -> App -> UI
		// then posts a property change to an object within the service thread object
		it("ThreadObject_Observable_Vector_ThreadObject", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<ThreadablesThreadObject>>(threadManager);
			auto childThreadObjectManager = std::make_shared<ThreadObjectManager<EmptyThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			// set up a waiter until we get a notification on the UI thread that something has been completed
			ReadyEvent e;
			auto subscribeHandle = threadObject_UI->emptyObjectArray->subscribe([&e](std::shared_ptr<EmptyThreadObject> newValue, size_t index, ObservableActionType action) {
				e.finished();
			});

			auto sizeBefore = threadObject_UI->emptyObjectArray->size();

			auto newChildObject = childThreadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 4);
			threadObject_Service->emptyObjectArray->insert(0, newChildObject);
			e.wait();

			AssertThat(threadObject_UI->emptyObjectArray->at(0), !IsNull());
			AssertThat(threadObject_UI->emptyObjectArray->size(), Equals(sizeBefore + 1));
			AssertThat(ThreadIds::ThreadId_UI, Equals(threadObject_UI->emptyObjectArray->at(0)->getThreadId()));
			AssertThat(ThreadIds::ThreadId_Service, Equals(threadObject_Service->emptyObjectArray->at(0)->getThreadId()));
			AssertThat(threadObject_UI->emptyObjectArray->at(0)->getId().instanceId, Equals(threadObject_Service->emptyObjectArray->at(0)->getId().instanceId));
			AssertThat(threadObject_UI->emptyObjectArray->at(0).get(), !Equals(threadObject_Service->emptyObjectArray->at(0).get()));
			AssertThat(threadObject_UI->emptyObjectArray->at(0)->getId().instanceId, Equals(threadObject_Service->emptyObjectArray->at(0)->getId().instanceId));

			threadObject_UI->emptyObjectArray->unsubscribe(subscribeHandle);
		});
		// sets up the threads like so:
		// Service -> App -> UI
		// And checks to make sure the initial values of each object are the same
		it("ThreadObject_Observable_InitialDefaultValue", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ThreadIds::ThreadId_App}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ThreadIds::ThreadId_UI}));
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_Service = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			AssertThat(threadObject_UI->intValue->get(), Equals(threadObject_Service->intValue->get()));

			auto threadObject_App = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			AssertThat(threadObject_UI->intValue->get(), Equals(threadObject_App->intValue->get()));
		});
		it("ThreadObject_RunOnPeer_Success", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			threadManager->getOrCreateThread(ThreadIds::ThreadId_App);
			threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);

			auto threadObjectManager = std::make_shared<ThreadObjectManager<PrimativesThreadObject>>(threadManager);
			auto threadObject_UI = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);
			auto threadObject_App = threadObjectManager->getOrCreateObject(ThreadIds::ThreadId_App, 0);

			threadObject_UI->intValue->set(0);
			threadObject_App->intValue->set(0);
			ReadyEvent e;
			threadObject_UI->runOnPeer(ThreadIds::ThreadId_App, [&e](std::shared_ptr<IThreadObject> peer) {
				auto appObject = std::static_pointer_cast<PrimativesThreadObject>(peer);
				appObject->intValue->set(4);
				e.finished();
			});

			e.wait();
			AssertThat(4, Equals(threadObject_App->intValue->get()));
		});
	});
});
}
}