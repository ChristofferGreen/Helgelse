#include <catch.hpp>

#include "Helgelse/GLFWVulkanSpace.hpp"
#include "PathSpace.hpp"


using namespace FSNG;
using namespace Helgelse;

TEST_CASE("Basic Vulkan") {
    PathSpaceTE space = PathSpace{};

    SECTION("Basic Window") {
        space.insert("/graphics", PathSpaceTE(GLFWVulkanSpace()));
        space.insert("/graphics/windows/main", CreateWindow{.title="Main", .fullscreen=false});
    }
}