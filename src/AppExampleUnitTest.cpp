// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ReadyEvent.h>
#include <ThreadIds.h>
#include <AppFactory.h>
#include <App.h>
// Test
#include "TestUtils.h"

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
    describe("AppExample", []() {
        it("ReadBusinessesAsync_VerifyNewBusinessIsOnUiThread", [&]() {
            auto app = AppFactory::getInstance().create();

            // first see if the business exists
            TestUtils::waitForAsync(
                app->isBusinessesPending,
                [&app]() {
                    app->readBusinessesAsync(0, 10, "Wit");
                });
            AssertThat(0, Equals((int)app->businesses->size()));

            // Since it doesn't exist, we should make it!
            TestUtils::waitForAsync(
                app->isCreateBusinessPending,
                [&app]() {
                    app->createBusinessAsync("Witness");
                });
            auto witness = app->createdBusiness->get();
            AssertThat(std::string("Witness"), Equals(witness->name->get()));

            auto similarToThreadilyBridge = witness->products->subscribe([](std::shared_ptr<Product> newValue, size_t index, threadily::ObservableActionType action) {
            });

            // Now let's re-query that business again and check that it's on the right thread
            TestUtils::waitForAsync(
                app->isBusinessesPending,
                [&app]() {
                    app->readBusinessesAsync(0, 10, "Wit");
                });
            AssertThat(1, Equals((int)app->businesses->size()));
            AssertThat((int)ThreadIds::ThreadId_UI, Equals((int)app->businesses->at(0)->getThreadId()));
        });
        it("ReadProductsAsync_bug_2017_03_18", [&]() {

            auto app = AppFactory::getInstance().create();

            // first see if the business exists
            TestUtils::waitForAsync(
                app->isBusinessesPending,
                [&app]() {
                    app->readBusinessesAsync(0, 10, "Wit");
                });
            AssertThat(0, Equals((int)app->businesses->size()));

            // Since it doesn't exist, we should make it!
            TestUtils::waitForAsync(
                app->isCreateBusinessPending,
                [&app]() {
                    app->createBusinessAsync("Witness");
                });
            auto witness = app->createdBusiness->get();
            AssertThat(std::string("Witness"), Equals(witness->name->get()));

            auto similarToThreadilyBridge = witness->products->subscribe([](std::shared_ptr<Product> newValue, size_t index, threadily::ObservableActionType action) {
            });

            // Even though we just created the business let's check it's product listings anyway
            TestUtils::waitForAsync(
                witness->isProductsPending,
                [&witness]() {
                    witness->readProductsAsync(0, 20, "");
                });
            AssertThat(0, Equals((int)witness->products->size()));

            // create a product
            TestUtils::waitForAsync(
                witness->isCreateProductPending,
                [&witness]() {
                    witness->createProductAsync("Name");
                });
            AssertThat(witness->createdProduct->get(), !Equals(nullptr));

            // Make sure the product exists
            TestUtils::waitForAsync(
                witness->isProductsPending,
                [&witness]() {
                    witness->readProductsAsync(0, 20, "");
                });
            AssertThat(1, Equals((int)witness->products->size()));
        });

        // https://github.com/k2snowman69/Threadily/issues/7
        it("BusinessVectorIncreasesRefCount_issue_7", [&]() {
            auto app = AppFactory::getInstance().create();

            // Create a business
            TestUtils::waitForAsync(
                app->isCreateBusinessPending,
                [&app]() {
                    app->createBusinessAsync("Witness");
                });
            auto witness = app->createdBusiness->get();
            AssertThat(std::string("Witness"), Equals(witness->name->get()));

            auto oldUseCount = witness.use_count();

            // Now query the business populating app->businesses
            TestUtils::waitForAsync(
                app->isBusinessesPending,
                [&app]() {
                    app->readBusinessesAsync(0, 10, "Wit");
                });
            // Should be 1 businesses in the system
            AssertThat(1, Equals((int)app->businesses->size()));
            // Expected the use count to increase by one as it should now be stored in the businesses listing
            AssertThat(oldUseCount + 1, Equals(witness.use_count()));
        });
    });
});
}
}