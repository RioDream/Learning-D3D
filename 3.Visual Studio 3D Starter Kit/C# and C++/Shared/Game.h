// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <memory>
#include <vector>

#include <wrl.h>
#include <DirectXMath.h>

#include "VSD3DStarter.h"
#include "Animation\Animation.h"
#include "GameBase.h"

ref class Game sealed : public GameBase
{
public:
    Game();

private:
    ~Game();

public:
	//重写接口
    virtual void CreateWindowSizeDependentResources() override;
    virtual void Initialize() override;
    virtual void Update(float timeTotal, float timeDelta) override;
    virtual void Render() override;
    
	//自己定义的函数
    Platform::String^ OnHitObject(int x, int y);
    void ToggleHitEffect(Platform::String^ object);
    void ChangeMaterialColor(Platform::String^ object, float r, float g, float b);

private:
	//自己的renderer 
    SkinnedMeshRenderer m_skinnedMeshRenderer;
	//所有mesh
    std::vector<VSD3DStarter::Mesh*> m_meshModels;
    DirectX::XMMATRIX m_teapotTransform;
    std::vector<float> m_time;
    float m_rotation;
};
