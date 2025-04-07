/**
 * @file MockGfxController.hpp
 * @author Christian Galvez
 * @brief Mock GfxController implemenation for unit tests.
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <gmock/gmock.h>
#include <GfxController.hpp>

class MockGfxController : public GfxController {
public:
    MOCK_METHOD(GfxResult<int>, init, (), (override));
    MOCK_METHOD(GfxResult<unsigned int>, generateBuffer, (unsigned int *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, generateTexture, (unsigned int *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, bindBuffer, (unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendBufferData, (size_t, void *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendTextureData, (unsigned int, unsigned int, TexFormat, void *), (override));
    MOCK_METHOD(GfxResult<int>, getShaderVariable, (unsigned int, const char *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, getProgramId, (uint), (override));
    MOCK_METHOD(GfxResult<unsigned int>, setProgram, (unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, loadShaders, (string, string), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendFloat, (unsigned int, float), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendFloatVector, (unsigned int, size_t, float *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, polygonRenderMode, (RenderMode), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendFloatMatrix, (unsigned int, size_t, float *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, sendInteger, (unsigned int, int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, bindTexture, (unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, initVao, (unsigned int *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, bindVao, (unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, setCapability, (GfxCapability, bool), (override));
    MOCK_METHOD(GfxResult<unsigned int>, deleteTextures, (unsigned int *), (override));
    MOCK_METHOD(GfxResult<unsigned int>, updateBufferData, (const vector<float> &, unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, setTexParam, (TexParam, TexVal), (override));
    MOCK_METHOD(GfxResult<unsigned int>, generateMipMap, (), (override));
    MOCK_METHOD(GfxResult<unsigned int>, enableVertexAttArray, (unsigned int, size_t), (override));
    MOCK_METHOD(GfxResult<unsigned int>, disableVertexAttArray, (unsigned int), (override));
    MOCK_METHOD(GfxResult<unsigned int>, drawTriangles, (unsigned int), (override));
    MOCK_METHOD(void, clear, (GfxClearMode), (override));
    MOCK_METHOD(void, update, (), (override));
    MOCK_METHOD(void, deleteBuffer, (unsigned int *), (override));
    MOCK_METHOD(void, deleteVao, (unsigned int *), (override));
};
