// std includes
#include <iostream>
#include <memory>
// Threadily includes
#include <ReadyEvent.h>
#include <ThreadIds.h>
#include <Observable.h>

namespace threadily
{
namespace test
{
class TestUtils {
public:
    static void waitForAsync(std::shared_ptr<threadily::Observable<bool>> isPending, std::function<void()> asyncMethod);
};
}
}