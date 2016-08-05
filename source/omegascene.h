#pragma once
#include "forwarddeclarations.h"

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaOsg/omegaOsg.h>
#undef foreach 

#include <osg/Group>
#include <osg/ref_ptr>




// #include "troengame.h"
class RenderPassListener;

namespace troen
{
    class SharedDataListener {
    public:
        virtual void commitSharedData(omega::SharedOStream& out) = 0;
        virtual void updateSharedData(omega::SharedIStream& in) = 0;
    };

    class RenderPassListener {
    public:
        virtual void onRender(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene) {}
    };

    class MyRenderPassListener : public omegaOsg::IOsgRenderPassListener
    {
    public:
        MyRenderPassListener(TroenGame* game) : m_game(game) {}
        virtual void onFrameFinished(omega::Renderer* client, const omega::DrawContext& context, omegaOsg::SceneView* scene);
    protected:
        TroenGame* m_game;
    };

    class TroenOmegaScene: public omega::EngineModule
    {
    public:
        TroenOmegaScene();

        virtual void initialize();
        virtual void update(const omega::UpdateContext& context);
        virtual void handleEvent(const omega::Event& evt);
        void updateOmegaCamera(const osg::Camera* cam);

        TroenGame* getTroenGame() { return m_troenGame;}
        void registerCorrectPath();

        void commitSharedData(omega::SharedOStream& out);
        void updateSharedData(omega::SharedIStream& in);

    private:

        omegaOsg::OsgModule* myOsg;
        omega::SceneNode* mySceneNode;
        omega::Actor* myInteractor;
        std::shared_ptr<MyRenderPassListener> myRenderPassListener;
        // TroenGame *m_troenGame;

        TroenGame* m_troenGame; 

    };

}