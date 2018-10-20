// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <Observable.h>
#include <ThreadIds.h>
#include <ThreadManager.h>
#include <ThreadObjectManager.h>
#include <ThreadablesThreadObject.h>

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
    describe("ThreadablesThreadObjectUnitTest", []() {
        it("ThreadablesThreadObject_Empty", [&]() {
			auto threadManager = std::make_shared<ThreadManager>();
			auto uiThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_UI);
			auto appThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_App, std::set<unsigned int>({ ThreadIds::ThreadId_UI }), nullptr);
			auto serviceThread = threadManager->getOrCreateThread(ThreadIds::ThreadId_Service, std::set<unsigned int>({ ThreadIds::ThreadId_App }), nullptr);

			auto threadablesThreadObjectManager = std::make_shared<ThreadObjectManager<ThreadablesThreadObject>>(threadManager);
			auto emptyObjectManager = std::make_shared<ThreadObjectManager<EmptyThreadObject>>(threadManager);

			auto obj_Service = threadablesThreadObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);
			auto obj_UI = threadablesThreadObjectManager->getOrCreateObject(ThreadIds::ThreadId_UI, 0);

			auto emptyObject = emptyObjectManager->getOrCreateObject(ThreadIds::ThreadId_Service, 0);

			auto handler = obj_UI->emptyObject->subscribe([emptyObject](std::shared_ptr<EmptyThreadObject> newValue){
				AssertThat(emptyObject->getId().instanceId, Equals(newValue->getId().instanceId));
			});

			AssertThat(obj_UI->emptyObject->get().get(), IsNull());
			obj_Service->emptyObject->set(emptyObject);

			obj_UI->emptyObject->unsubscribe(handler);
		});
	});
});
}
}