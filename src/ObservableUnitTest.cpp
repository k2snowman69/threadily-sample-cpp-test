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
    describe("ObservableUnitTest", []() {
        it("Observable_Int_Insert_1", [&]() {
            auto observable = Observable<int>();
            AssertThat(0, Equals(observable.get()));

            observable.set(5);
            AssertThat(5, Equals(observable.get()));
        });
        // Bug fix - If we removed a subscription during callbacks, we would get an error
        it("Observable_Int_SubscriptionBug_2017_02_26", [&]() {
            auto observable = Observable<int>();
            AssertThat(0, Equals(observable.get()));

            threadily::ReadyEvent r1, r2, r3;

            auto subscription = observable.subscribe([&r1](int newValue) {
                r1.finished();
            });
            auto subscription1 = observable.subscribe([&observable, &subscription, &r2](int newValue) {
                observable.unsubscribe(subscription);
                r2.finished();
            });
            auto subscription2 = observable.subscribe([&r3](int newValue) {
                r3.finished();
            });

            observable.set(5);
            r1.wait();
            r2.wait();
            r3.wait();

            AssertThat(5, Equals(observable.get()));
        });
        it("Observable_Int_SubscriptionBug_AddDuringSubscribe", [&]() {
            auto observable = Observable<int>();
            AssertThat(0, Equals(observable.get()));

            threadily::ReadyEvent r1, r2, r3;

            bool r4Hit = false;

            std::shared_ptr<threadily::ISubscribeHandle> subscription3;
            auto subscription = observable.subscribe([&r1](int newValue) {
                r1.finished();
            });
            auto subscription1 = observable.subscribe([&observable, &subscription, &r2, &r4Hit, &subscription3](int newValue) {
                subscription3 = observable.subscribe([&r4Hit](int newValue) {
                    r4Hit = true;
                });
                r2.finished();
            });
            auto subscription2 = observable.subscribe([&r3](int newValue) {
                r3.finished();
            });

            observable.set(5);
            r1.wait();
            r2.wait();
            r3.wait();
            
            AssertThat(r4Hit, IsFalse());

            AssertThat(5, Equals(observable.get()));
        });
        it("Observable_Ptr_Int_Insert_1", [&]() {
            auto observable = Observable<std::shared_ptr<int>>();
            AssertThat(nullptr == observable.get(), IsTrue());

            observable.set(std::make_shared<int>(5));
            AssertThat(nullptr == observable.get(), IsFalse());
            AssertThat(5, Equals(*observable.get()));
        });
    });
});
}
}