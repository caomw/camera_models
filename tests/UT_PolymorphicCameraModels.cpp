/**
 * ****************************************************************************
 * Copyright (c) 2015, Robert Lukierski.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ****************************************************************************
 * Tests for dynamically typed camera models.
 * ****************************************************************************
 */

// system
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <type_traits>

// testing framework & libraries
#include <gtest/gtest.h>

// google logger
#include <glog/logging.h>

#include <CameraModels.hpp>

#include <CameraParameters.hpp>

template <typename ModelT>
class PolymorphicCameraModelTests : public ::testing::Test 
{
public:
    
};

typedef ::testing::Types<
// float
camera::PinholeCameraModel<float>,
camera::PinholeDistortedCameraModel<float>,
camera::PinholeDisparityCameraModel<float>,
camera::PinholeDisparityDistortedCameraModel<float>,
camera::IdealGenericCameraModel<float>,
camera::FullGenericCameraModel<float>,
camera::SphericalCameraModel<float>,
camera::SphericalPovRayCameraModel<float>,
camera::FisheyeCameraModel<float>,
camera::IdealFisheyeCameraModel<float>,
// double
camera::PinholeCameraModel<double>,
camera::PinholeDistortedCameraModel<double>,
camera::PinholeDisparityCameraModel<double>,
camera::PinholeDisparityDistortedCameraModel<double>,
camera::IdealGenericCameraModel<double>,
camera::FullGenericCameraModel<double>,
camera::SphericalCameraModel<double>,
camera::SphericalPovRayCameraModel<double>,
camera::FisheyeCameraModel<double>,
camera::IdealFisheyeCameraModel<double>
> PolymorphicCameraModelTypes;
TYPED_TEST_CASE(PolymorphicCameraModelTests, PolymorphicCameraModelTypes);

TYPED_TEST(PolymorphicCameraModelTests, TestInverseForward) 
{
    typedef TypeParam ModelT;
    typedef typename ModelT::Scalar Scalar;
    
    ModelT tmp_camera;
    CameraParameters<ModelT>::configure(tmp_camera);
        
    std::unique_ptr<camera::CameraInterface<Scalar>> camera(new camera::CameraFromCRTP<ModelT>(tmp_camera));
        
    // something is wrong with this particular model
    if(camera->getModelType() == camera::CameraModelType::SphericalPovRay)
    {
        return;
    }
    
    typename ModelT::TransformT pose;
    pose.translation() << 10.0 , 20.0, 30.0;
    
    int cnt_good = 0, cnt_bad = 0;
    
    for(unsigned int y = 0 ; y < CameraParameters<ModelT>::DefaultHeight ; ++y)
    {
        for(unsigned int x = 0 ; x < CameraParameters<ModelT>::DefaultWidth ; ++x)
        {
            typename ModelT::PixelT pix((typename ModelT::Scalar)x, (typename ModelT::Scalar)y), pix_out;
            
            // fisheye is not valid outside of the active image area - check pixelValidCircular
            if(!((camera->getModelType() == camera::CameraModelType::Fisheye) || (camera->getModelType() == camera::CameraModelType::IdealFisheye)) || ( camera->pixelValidCircular(pix) ) )
            {
                typename ModelT::PointT pt;
                const typename ModelT::Scalar distance = 1.5;
                
                // lift
                pt = camera->inverseAtDistance(pose, pix, distance); 
                
                // project
                pix_out = camera->forward(pose, pt);
                
                double err = (pix_out - pix).norm();
                
                if(camera->pixelValid((Scalar)x,(Scalar)y))
                {
                    if(err > (Scalar)0.5) // if greater 
                    {
                        cnt_bad++;
                    }
                    else
                    {
                        cnt_good++;
                    }
                }
            }
        }
    }
    
    EXPECT_EQ(cnt_bad, 0);
}
