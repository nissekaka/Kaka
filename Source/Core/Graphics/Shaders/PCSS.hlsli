
//#define BLOCKER_SEARCH_NUM_SAMPLES 16
//#define PCF_NUM_SAMPLES 16
//#define NEAR_PLANE 9.5
//#define LIGHT_WORLD_SIZE .5
//#define LIGHT_FRUSTUM_WIDTH 3.75
//// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT
//#define LIGHT_SIZE_UV (LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH)

//Texture2D<float> tDepthMap;

//SamplerComparisonState shadowSplrCmp : register(s2);


//float PenumbraSize(float zReceiver, float zBlocker) //Parallel plane estimation
//{
//    return (zReceiver - zBlocker) / zBlocker;
//}
//void FindBlocker(out float avgBlockerDepth, out float numBlockers, float2 uv, float zReceiver)
//{
////This uses similar triangles to compute what
////area of the shadow map we should search
//    float searchWidth = LIGHT_SIZE_UV * (zReceiver - NEAR_PLANE) / zReceiver;
//    float blockerSum = 0;
//    numBlockers = 0;
//    for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i)
//    {
//        float shadowMapDepth = tDepthMap.SampleLevel(shadowSplr, uv + poissonDisk[i] * searchWidth, 0);
//        if (shadowMapDepth < zReceiver)
//        {
//            blockerSum += shadowMapDepth;
//            numBlockers++;
//        }
//    }
//    avgBlockerDepth = blockerSum / numBlockers;
//}
//float PCF_Filter(float2 uv, float zReceiver, float filterRadiusUV)
//{
//    float sum = 0.0f;
//    for (int i = 0; i < PCF_NUM_SAMPLES; ++i)
//    {
//        float2 offset = poissonDisk[i] * filterRadiusUV;
//        sum += tDepthMap.SampleCmpLevelZero(shadowSplrCmp, uv + offset, zReceiver);
//    }
//    return sum / PCF_NUM_SAMPLES;
//}

//float PCSS(Texture2D shadowMapTex, float4 coords)
//{
//    float2 uv = coords.xy;
//    float zReceiver = coords.z; // Assumed to be eye-space z in this code
//// STEP 1: blocker search
//    float avgBlockerDepth = 0;
//    float numBlockers = 0;
//    FindBlocker(avgBlockerDepth, numBlockers, uv, zReceiver);
//    if (numBlockers < 1)
////There are no occluders so early out (this saves filtering)
//        return 1.0f;
//// STEP 2: penumbra size
//    float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);
//    float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV * NEAR_PLANE / coords.z;
//// STEP 3: filtering
//    return PCF_Filter(uv, zReceiver, filterRadiusUV);
//}