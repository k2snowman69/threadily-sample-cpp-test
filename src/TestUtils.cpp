#include "TestUtils.h"


namespace threadily
{
namespace test
{
void TestUtils::waitForAsync(std::shared_ptr<threadily::Observable<bool>> isPending, std::function<void()> asyncMethod)
{
    threadily::ReadyEvent ready;
    auto subscriptionHandle = isPending->subscribe([&ready](bool isPending) {
        if (!isPending)
        {
            ready.finished();
        }
    });
    asyncMethod();
    ready.wait();
    isPending->unsubscribe(subscriptionHandle);
}
}
}