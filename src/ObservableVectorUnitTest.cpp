// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ReadyEvent.h>
#include <Observable.h>
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
    describe("ObservableVectorUnitTest", []() {
        it("Observable_Vector_Int_Insert_1", [&]() {
			auto observableVector = Observable<std::vector<int>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(0, 1);
			observableVector.insert(1, 2);
			observableVector.insert(0, 0);
			AssertThat((size_t)3, Equals(observableVector.size()));

			AssertThat(0, Equals(observableVector.at(0)));
			AssertThat(1, Equals(observableVector.at(1)));
			AssertThat(2, Equals(observableVector.at(2)));
		});

        it("Observable_Vector_Int_Insert_2", [&]() {
			auto observableVector = Observable<std::vector<int>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(2, 2);
			AssertThat((size_t)3, Equals(observableVector.size()));

			AssertThat(0, Equals(observableVector.at(0)));
			AssertThat(0, Equals(observableVector.at(1)));
			AssertThat(2, Equals(observableVector.at(2)));
		});

        it("Observable_Vector_Int_Set", [&]() {
			auto observableVector = Observable<std::vector<int>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(2, 2);
			observableVector.set(1, 1);
			observableVector.set(0, 0);

			AssertThat((size_t)3, Equals(observableVector.size()));
			AssertThat(0, Equals(observableVector.at(0)));
			AssertThat(1, Equals(observableVector.at(1)));
			AssertThat(2, Equals(observableVector.at(2)));
		});

        it("Observable_Vector_Int_Set_OutOfOrder", [&]() {
			auto observableVector = Observable<std::vector<int>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.set(2, 2);
			observableVector.set(1, 1);
			observableVector.set(0, 0);

			AssertThat((size_t)3, Equals(observableVector.size()));
			AssertThat(0, Equals(observableVector.at(0)));
			AssertThat(1, Equals(observableVector.at(1)));
			AssertThat(2, Equals(observableVector.at(2)));
		});
		
        it("Observable_Vector_Ptr_Insert_1", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(0, std::make_shared<int>(1));
			observableVector.insert(1, std::make_shared<int>(2));
			observableVector.insert(0, std::make_shared<int>(0));
			AssertThat((size_t)3, Equals(observableVector.size()));

			AssertThat(0, Equals(*(observableVector.at(0).get())));
			AssertThat(1, Equals(*(observableVector.at(1).get())));
			AssertThat(2, Equals(*(observableVector.at(2).get())));
		});

        it("Observable_Vector_Ptr_Insert_2", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(2, std::make_shared<int>(2));
			AssertThat((size_t)3, Equals(observableVector.size()));

			AssertThat(observableVector.at(0).get(), IsNull());
			AssertThat(observableVector.at(1).get(), IsNull());
			AssertThat(2, Equals(*(observableVector.at(2).get())));
		});

        it("Observable_Vector_Ptr_Set", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.insert(2, std::make_shared<int>(2));
			observableVector.set(1, std::make_shared<int>(1));
			observableVector.set(0, std::make_shared<int>(0));

			AssertThat((size_t)3, Equals(observableVector.size()));
			AssertThat(0, Equals(*(observableVector.at(0).get())));
			AssertThat(1, Equals(*(observableVector.at(1).get())));
			AssertThat(2, Equals(*(observableVector.at(2).get())));
		});

        it("Observable_Vector_Ptr_Set_OutOfOrder", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			observableVector.set(2, std::make_shared<int>(2));
			observableVector.set(1, std::make_shared<int>(1));
			observableVector.set(0, std::make_shared<int>(0));

			AssertThat((size_t)3, Equals(observableVector.size()));
			AssertThat(0, Equals(*(observableVector.at(0).get())));
			AssertThat(1, Equals(*(observableVector.at(1).get())));
			AssertThat(2, Equals(*(observableVector.at(2).get())));
		});

        it("Observable_Vector_Ptr_Subscription_Insert", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			auto valueAdded = std::make_shared<int>(4);

			auto subscribe = observableVector.subscribe([valueAdded](std::shared_ptr<int> newValue, size_t index, ObservableActionType action) {
				AssertThat(valueAdded, Equals(newValue));
				AssertThat((size_t)2, Equals(index));
				AssertThat(ObservableActionType::Insert, Equals(action));
			});

			observableVector.insert(2, valueAdded);

			AssertThat((size_t)3, Equals(observableVector.size()));

			observableVector.unsubscribe(subscribe);
		});

        it("Observable_Vector_Ptr_Subscription_Remove", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			bool isDeleted = false;
			auto valueAdded = std::make_shared<int>(4);

			auto subscribe = observableVector.subscribe([valueAdded, &isDeleted](std::shared_ptr<int> newValue, size_t index, ObservableActionType action) {
				if (ObservableActionType::Erase == action)
				{
					AssertThat(valueAdded, Equals(newValue));
					AssertThat((size_t)2, Equals(index));
					isDeleted = true;
				}
			});

			observableVector.set(0, std::make_shared<int>(0));
			observableVector.set(1, std::make_shared<int>(2));
			observableVector.set(2, valueAdded);

			AssertThat((size_t)3, Equals(observableVector.size()));

			observableVector.erase(2);

			AssertThat((size_t)2, Equals(observableVector.size()));
			AssertThat(isDeleted, IsTrue());

			observableVector.unsubscribe(subscribe);
		});

        it("Observable_Vector_Ptr_Subscription_Update", [&]() {
			auto observableVector = Observable<std::vector<std::shared_ptr<int>>>();
			AssertThat((size_t)0, Equals(observableVector.size()));

			bool isUpdated = false;
			auto valueAdded = std::make_shared<int>(4);
			auto valueUpdated = std::make_shared<int>(6);

			auto subscribe = observableVector.subscribe([valueUpdated, &isUpdated](std::shared_ptr<int> newValue, size_t index, ObservableActionType action) {
				if (ObservableActionType::Set == action)
				{
					AssertThat(valueUpdated, Equals(newValue));
					AssertThat((size_t)2, Equals(index));
					AssertThat(ObservableActionType::Set, Equals(action));
					isUpdated = true;
				}
			});

			observableVector.set(0, std::make_shared<int>(0));
			observableVector.set(1, std::make_shared<int>(2));
			observableVector.set(2, valueAdded);

			AssertThat((size_t)3, Equals(observableVector.size()));

			observableVector.set(2, valueUpdated);

			AssertThat((size_t)3, Equals(observableVector.size()));
			AssertThat(isUpdated, IsTrue());

			observableVector.unsubscribe(subscribe);
		});
	});
});
}
}