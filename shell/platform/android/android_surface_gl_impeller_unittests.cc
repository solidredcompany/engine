#include "flutter/shell/platform/android/android_surface_gl_impeller.h"
#include "flutter/shell/platform/android/jni/jni_mock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

using ::testing::_;
using ::testing::AllOf;
using ::testing::ByMove;
using ::testing::Field;
using ::testing::Matcher;
using ::testing::Return;

using ::impeller::egl::Config;
using ::impeller::egl::ConfigDescriptor;

namespace {
class MockDisplay : public impeller::egl::Display {
 public:
  MOCK_CONST_METHOD0(IsValid, bool());
  MOCK_CONST_METHOD1(ChooseConfig, std::unique_ptr<Config>(ConfigDescriptor));
};
}  // namespace

TEST(AndroidSurfaceGLImpeller, MSAAFirstAttempt) {
  auto context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto jni = std::make_shared<JNIMock>();
  auto display = std::make_unique<MockDisplay>();
  EXPECT_CALL(*display, IsValid).WillRepeatedly(Return(true));
  auto first_result = std::make_unique<Config>(ConfigDescriptor(), EGLConfig());
  auto second_result =
      std::make_unique<Config>(ConfigDescriptor(), EGLConfig());
  EXPECT_CALL(
      *display,
      ChooseConfig(Matcher<ConfigDescriptor>(AllOf(
          Field(&ConfigDescriptor::samples, impeller::egl::Samples::kFour),
          Field(&ConfigDescriptor::surface_type,
                impeller::egl::SurfaceType::kWindow)))))
      .WillOnce(Return(ByMove(std::move(first_result))));
  EXPECT_CALL(*display, ChooseConfig(Matcher<ConfigDescriptor>(
                            Field(&ConfigDescriptor::surface_type,
                                  impeller::egl::SurfaceType::kPBuffer))))
      .WillOnce(Return(ByMove(std::move(second_result))));
  ON_CALL(*display, ChooseConfig(_))
      .WillByDefault(Return(ByMove(std::unique_ptr<Config>())));
  auto surface = std::make_unique<AndroidSurfaceGLImpeller>(context, jni,
                                                            std::move(display));
  ASSERT_TRUE(surface);
}

TEST(AndroidSurfaceGLImpeller, FallbackForEmulator) {
  auto context =
      std::make_shared<AndroidContext>(AndroidRenderingAPI::kSoftware);
  auto jni = std::make_shared<JNIMock>();
  auto display = std::make_unique<MockDisplay>();
  EXPECT_CALL(*display, IsValid).WillRepeatedly(Return(true));
  std::unique_ptr<Config> first_result;
  auto second_result =
      std::make_unique<Config>(ConfigDescriptor(), EGLConfig());
  auto third_result = std::make_unique<Config>(ConfigDescriptor(), EGLConfig());
  EXPECT_CALL(
      *display,
      ChooseConfig(Matcher<ConfigDescriptor>(AllOf(
          Field(&ConfigDescriptor::samples, impeller::egl::Samples::kFour),
          Field(&ConfigDescriptor::surface_type,
                impeller::egl::SurfaceType::kWindow)))))
      .WillOnce(Return(ByMove(std::move(first_result))));
  EXPECT_CALL(
      *display,
      ChooseConfig(Matcher<ConfigDescriptor>(
          AllOf(Field(&ConfigDescriptor::samples, impeller::egl::Samples::kOne),
                Field(&ConfigDescriptor::surface_type,
                      impeller::egl::SurfaceType::kWindow)))))
      .WillOnce(Return(ByMove(std::move(second_result))));
  EXPECT_CALL(*display, ChooseConfig(Matcher<ConfigDescriptor>(
                            Field(&ConfigDescriptor::surface_type,
                                  impeller::egl::SurfaceType::kPBuffer))))
      .WillOnce(Return(ByMove(std::move(third_result))));
  ON_CALL(*display, ChooseConfig(_))
      .WillByDefault(Return(ByMove(std::unique_ptr<Config>())));
  auto surface = std::make_unique<AndroidSurfaceGLImpeller>(context, jni,
                                                            std::move(display));
  ASSERT_TRUE(surface);
}
}  // namespace testing
}  // namespace flutter
