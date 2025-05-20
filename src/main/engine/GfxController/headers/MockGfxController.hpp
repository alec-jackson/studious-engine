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
#include <string>
#include <vector>
#include <GfxController.hpp>

class MockGfxController : public GfxController {
 public:
    MOCK_METHOD(GfxResult<int>, init, (), (override));
    MOCK_METHOD(GfxResult<uint>, generateBuffer, (uint *), (override));
    MOCK_METHOD(GfxResult<uint>, generateTexture, (uint *), (override));
    MOCK_METHOD(GfxResult<uint>, bindBuffer, (uint), (override));
    MOCK_METHOD(GfxResult<uint>, sendBufferData, (size_t, void *), (override));
    MOCK_METHOD(GfxResult<uint>, sendTextureData, (uint, uint, TexFormat, void *), (override));
    MOCK_METHOD(GfxResult<uint>, sendTextureData3D, (int, int, int, uint, uint, TexFormat, void *), (override));
    MOCK_METHOD(GfxResult<int>, getShaderVariable, (uint, const char *), (override));
    MOCK_METHOD(GfxResult<uint>, getProgramId, (string), (override));
    MOCK_METHOD(GfxResult<uint>, setProgram, (uint), (override));
    MOCK_METHOD(GfxResult<uint>, loadShaders, (string, string, string), (override));
    MOCK_METHOD(GfxResult<uint>, sendFloat, (uint, float), (override));
    MOCK_METHOD(GfxResult<uint>, sendFloatVector, (uint, size_t, float *), (override));
    MOCK_METHOD(GfxResult<uint>, polygonRenderMode, (RenderMode), (override));
    MOCK_METHOD(GfxResult<uint>, sendFloatMatrix, (uint, size_t, float *), (override));
    MOCK_METHOD(GfxResult<uint>, sendInteger, (uint, int), (override));
    MOCK_METHOD(GfxResult<uint>, bindTexture, (uint, GfxTextureType), (override));
    MOCK_METHOD(GfxResult<uint>, initVao, (uint *), (override));
    MOCK_METHOD(GfxResult<uint>, bindVao, (uint), (override));
    MOCK_METHOD(GfxResult<uint>, setCapability, (GfxCapability, bool), (override));
    MOCK_METHOD(GfxResult<uint>, deleteTextures, (uint *), (override));
    MOCK_METHOD(GfxResult<uint>, updateBufferData, (const vector<float> &, uint), (override));
    MOCK_METHOD(GfxResult<uint>, setTexParam, (TexParam, TexVal, GfxTextureType), (override));
    MOCK_METHOD(GfxResult<uint>, generateMipMap, (), (override));
    MOCK_METHOD(GfxResult<uint>, enableVertexAttArray, (uint, int, size_t, void *), (override));
    MOCK_METHOD(GfxResult<uint>, setVertexAttDivisor, (uint, uint), (override));
    MOCK_METHOD(GfxResult<uint>, disableVertexAttArray, (uint), (override));
    MOCK_METHOD(GfxResult<uint>, drawTriangles, (uint), (override));
    MOCK_METHOD(GfxResult<uint>, drawTrianglesInstanced, (uint, uint), (override));
    MOCK_METHOD(GfxResult<uint>, allocateTexture3D, (TexFormat, uint, uint, uint), (override));
    MOCK_METHOD(void, clear, (GfxClearMode), (override));
    MOCK_METHOD(void, update, (), (override));
    MOCK_METHOD(void, deleteBuffer, (uint *), (override));
    MOCK_METHOD(void, deleteVao, (uint *), (override));
    // virtual void setBgColor(float r, float g, float b) = 0;
    MOCK_METHOD(void, setBgColor, (float r, float g, float b), (override));
};
