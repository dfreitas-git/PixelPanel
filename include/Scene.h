
#pragma once

#include "WandProtocol.h"
#include "PixelFramebuffer.h"

// This is our base class that individual scenes will inherit and modify
class Scene {
public:

    virtual const char* name() const = 0;  // the name of the sceme

    virtual ~Scene() = default;
    virtual void begin() {}

    // virtual void processPose(...) {} says that every scene may override processPose(). 
    // If a particular scene doesn't need pose information, the empty base-class implementation is perfectly acceptable.
    // virtual void render(...) = 0; says that every concrete scene must provide its own 
    // implementation of render(). This is called a pure virtual function.

    virtual void processPose(const WandPacket& packet) {}
    virtual void processGesture(const GestureEvent& event) {}
    virtual void render(PixelFramebuffer& framebuffer) = 0;

    // What to do when the wand button is pressed once
    virtual void onSingleClick() {}

    // Preferred diffuser distance for this scene.
    // 0 = minimum blur, 100 = maximum blur.
    virtual int preferredDiffuserPosition() const { return 0; }
};
