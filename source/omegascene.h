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

namespace troen
{


    class TroenOmegaScene: public omega::EngineModule
    {
    public:
        TroenOmegaScene(): omega::EngineModule("TroenOmegaScene")
        {
            myOsg = new omegaOsg::OsgModule();
            omega::ModuleServices::addModule(myOsg); 
        }

        virtual void initialize();
        virtual void update(const omega::UpdateContext& context);
        virtual void handleEvent(const omega::Event& evt) {}

        void setRootNode(osg::ref_ptr<osg::Group> root);
        void setSceneNode(osg::ref_ptr<osg::Group> scene);

    private:

        omegaOsg::OsgModule* myOsg;
        omega::SceneNode* mySceneNode;
        omega::Actor* myInteractor;
        osg::ref_ptr<osg::Group> m_root;
        osg::ref_ptr<osg::Group> m_scene;



    };

}