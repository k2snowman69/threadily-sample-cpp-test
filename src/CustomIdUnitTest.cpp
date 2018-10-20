// Main test framework
#include <bandit/bandit.h>
// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include "ThreadIds.h"
#include "ReadyEvent.h"
#include "ThreadManager.h"
#include "ProductManager.h"
#include "ProductId.h"
#include "Product.h"

using namespace snowhouse;
using namespace bandit;

namespace threadily
{
namespace test
{

go_bandit([]() {
    describe("CustomIdUnitTest", []() {
        it("GetOrCreateReturnsCustomId", [&]() {
            auto threadManager = std::make_shared<ThreadManager>();
            auto manager = std::make_shared<ProductManager>(threadManager);

            // Create an ID object to keep referencing
            auto id = ProductId(10, 11);
            AssertThat(10, Equals(id.businessId));
            AssertThat(11, Equals(id.productId));

            // create the product with the id requested
            auto product_UI = manager->getOrCreateObject(
                ThreadIds::ThreadId_UI, 
                id.businessId, 
                id.productId);

            // verify that the ID was set properly
            AssertThat(id.businessId, Equals(product_UI->getId().businessId));
            AssertThat(id.productId, Equals(product_UI->getId().productId));

            // now lets get a sibling object
            ReadyEvent re;
            product_UI->runOnPeer(ThreadIds::ThreadId_Service, [&re, &id](std::shared_ptr<IThreadObject> peer) {
                auto product_Service = std::static_pointer_cast<Product>(peer);

                // verify that the ID was set properly
                AssertThat(id.businessId, Equals(product_Service->getId().businessId));
                AssertThat(id.productId, Equals(product_Service->getId().productId));

                // Now see if editing the old id will edit this guys Id
                id.businessId += 20;
                AssertThat(id.businessId, !Equals(product_Service->getId().businessId));
                AssertThat(id.productId, Equals(product_Service->getId().productId));

                re.finished();
            });
            re.wait();
        });
    });
});
}
}