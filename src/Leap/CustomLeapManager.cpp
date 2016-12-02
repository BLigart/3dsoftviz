#include "Leap/CustomLeapManager.h"

#include "Viewer/CameraManipulator.h"
#include "Viewer/CoreGraph.h"
#include "Layout/LayoutThread.h"
#include "Layout/FRAlgorithm.h"
#include "HandModule/HandPalm.h"

#include <easylogging++.h>
#include <math.h>

const float forwardOffset = 4.0f;
const float downOffset = -1.0f;

Leap::CustomLeapManager::CustomLeapManager( Vwr::CameraManipulator* cameraManipulator, Layout::LayoutThread* layout,
        Vwr::CoreGraph* coreGraph , osg::ref_ptr<osg::Group> handsGroup )
    :cameraManipulator( cameraManipulator ), layout( layout ), coreGraph( coreGraph ), handsGroup( handsGroup )
{
    this->coreGraph->getCamera()->getViewMatrixAsLookAt( this->eye, this->center, this->up );
    arMode = false;
    //init handPalms here
    if ( this->handsGroup != nullptr ) {
        arMode = true;
        HandPalm* rightPalm = new HandPalm( 0.1f, handsGroup, 1 );
        HandPalm* leftPalm = new HandPalm( 0.1f, handsGroup, 2 );

        rightPalm->setMatrix( osg::Matrix::translate( this->center[0]-0.5,this->center[1],this->center[2] ) );
        leftPalm->setMatrix( osg::Matrix::translate( this->center[0]+0.5,this->center[1],this->center[2] ) );

    }

}

Leap::CustomLeapManager::~CustomLeapManager( void )
{
    //remove hands nodes here
    if ( handsGroup ) {
        handsGroup->removeChildren( 0,handsGroup->getNumChildren() );
    }
}

void Leap::CustomLeapManager::enableCameraMovement( Movement direction )
{
    this->cameraManipulator->enableCameraMovement( static_cast<Vwr::CameraManipulator::Movement>( direction ) );
}

void Leap::CustomLeapManager::disableCameraMovement()
{
    this->cameraManipulator->disableCameraMovement();
}

void Leap::CustomLeapManager::rotateCamera( float py0, float px0, double throwScale, float py1, float px1 )
{
    this->cameraManipulator->rotateCamera( py0,px0,throwScale,py1, px1 );
}

//jurik
void Leap::CustomLeapManager::graphRotateSwipe( int swipeDirection )
{
    //direction -1 = LEFT, 1 = RIGHT
    switch ( swipeDirection ) {
        case -1: {
            coreGraph->rotateGraph( swipeDirection );
            break;
        }
        case 1: {
            coreGraph->rotateGraph( swipeDirection );
            break;
        }
        default:
            break;
    }
}

void Leap::CustomLeapManager::rotateArucoLeft()
{
    coreGraph->rotateGraph( 1 );
}

void Leap::CustomLeapManager::rotateArucoRight()
{
    coreGraph->rotateGraph( -1 );
}

void Leap::CustomLeapManager::scaleEdgesUp()
{

    float distance = layout->getAlg()->getMaxDistance();

    layout->pause();
    coreGraph->setNodesFreezed( true );
    layout->getAlg()->setMaxDistance( distance * 1.02f );
    coreGraph->scaleGraphToBase();
    coreGraph->setNodesFreezed( false );
    layout->play();
}

void Leap::CustomLeapManager::scaleEdgesDown()
{

    float distance = layout->getAlg()->getMaxDistance();

    layout->pause();
    coreGraph->setNodesFreezed( true );
    layout->getAlg()->setMaxDistance( distance * 0.98f );
    coreGraph->scaleGraphToBase();
    coreGraph->setNodesFreezed( false );
    layout->play();
}

void Leap::CustomLeapManager::scaleNodes( bool scaleUp )
{
    if ( scaleUp ) {
        coreGraph->scaleNodes( true );
    }
    else {
        coreGraph->scaleNodes( false );
    }
}


void Leap::CustomLeapManager::updateHands( Leap::Hand leftHand, Leap::Hand rightHand )
{


    HandPalm* leftPalm = nullptr;
    HandPalm* rightPalm = nullptr;
    this->coreGraph->getCamera()->getViewMatrixAsLookAt( this->eye, this->center, this->up );
    this->direction = this->center - this->eye;

    if ( this->handsGroup != NULL ) {
        // update lavej ruky
        if ( leftHand.isValid() ) {
            Leap::Vector lVector = Leap::Vector( this->center[0]+0.5,this->center[1],this->center[2] );
            //ziskanie pozicie dlane
            lVector = leftHand.palmPosition();
            //0 a 3 z dovodu ze v grupe je palmNode, fingerGroup, palmNode, fingerGroup
            HandPalm* leftPalm = static_cast<HandPalm*>( handsGroup->getChild( 3 ) );

            leftPalm->setMatrix(
                osg::Matrix::translate( static_cast<double>(this->center[0]) + this->direction[0] + static_cast<double>( lVector.x )/100.0,
                                        static_cast<double>(this->center[1])+this->direction[1]*1 +static_cast<double>( -lVector.z )/100.0,
                                        static_cast<double>(this->center[2])+this->direction[2] +static_cast<double>( lVector.y )/100.0 ));
            // update prstov lavej ruky
            this->updateFingers( leftPalm, leftHand.fingers() );
            // update kosti medzi prstamu
            this->updateInterFingerBones( leftPalm->interFingerBoneGroup, leftHand.fingers() );
        }
        // update pravej ruky
        if ( rightHand.isValid() ) {
            Leap::Vector rVector = Leap::Vector( this->center[0]-0.5,this->center[1],this->center[2] );
            //ziskanie pozicie dlane
            rVector = rightHand.palmPosition();
            //0 a 3 z dovodu ze v grupe je palmNode, fingerGroup, palmNode, fingerGroup
            HandPalm* rightPalm = static_cast<HandPalm*>( handsGroup->getChild( 0 ) );

            rightPalm->setMatrix(
                osg::Matrix::translate( this->center[0]+this->direction[0] +static_cast<double>( rVector.x )/100.0,
                                        this->center[1]+this->direction[1]*1 +static_cast<double>( -rVector.z )/100.0,
                                        this->center[2] +this->direction[2]+static_cast<double>( rVector.y )/100.0 ));
            // update prstov pravej ruky
            this->updateFingers( rightPalm, rightHand.fingers() );
            // update kosti medzi prstamu
            this->updateInterFingerBones( rightPalm->interFingerBoneGroup, rightHand.fingers() );
        }

        LOG (INFO) << "direction "+ std::to_string(this->direction[0]) + " " +
                std::to_string(this->direction[1]) + " " +
                std::to_string(this->direction[2]);
        LOG (INFO) << "eye "+ std::to_string(this->eye[0]) + " " +
                std::to_string(this->eye[1]) + " " +
                std::to_string(this->eye[2]);
        LOG (INFO) << "center "+ std::to_string(this->center[0]) + " " +
                std::to_string(this->center[1]) + " " +
                std::to_string(this->center[2]);


    }

//    osg::RefMatrixd* boneMatrix = new osg::RefMatrixd();
//    boneMatrix->makeIdentity();
//    ->preMult( osg::Matrix::rotate( osg::Vec3f( 0.0f,0.0f,1.0f ) ,
//                             osg::Vec3f( static_cast<osg::Vec3f::value_type>( dirVector.x/100.0f ),
//                                         static_cast<osg::Vec3f::value_type>( -( dirVector.z/100.0f ) ) ,
//                                         static_cast<osg::Vec3f::value_type>( dirVector.y/100.0f ) ) ) );
//    this->handsGroup->asTransform()->t
}

void Leap::CustomLeapManager::updateFingers( HandPalm* palm, Leap::FingerList fingers )
{
    int i = 0;
    // update jointov vsetkych prstov
    for ( i = 0; i < 5; i++ ) {
        updateJoints( static_cast<osg::Group*>( palm->fingerGroup->getChild( static_cast<unsigned int>( i ) )->asGroup() ),
                      fingers[i], i );
    }
    // update kosti vsetkych prstov
    for ( i = 5; i < 10; i++ ) {
        updateFingerBones( static_cast<osg::Group*>( palm->fingerGroup->getChild( static_cast<unsigned int>( i ) )->asGroup() ),
                           fingers[i-5], i-5 );
    }
}


void Leap::CustomLeapManager::updateJoints( osg::Group* fingerJointGroup, Leap::Finger fingerLeap, int fingerPosition )
{
    // vykreslenie klbov zapastia ( klby v scene su ratene 0,1,2,3 s tym ze 4-ty je klb zapestia )
    if ( fingerPosition !=  2 && fingerPosition!= 3 ) {
        Joint* joint = static_cast<Joint*>( fingerJointGroup->getChild( 4 ) );
        Leap::Vector posVector = Leap::Vector( 0.0f,0.0f,0.0f );
        if ( fingerLeap.bone( static_cast<Leap::Bone::Type>( 0 ) ).isValid() ) {
            posVector = fingerLeap.bone( static_cast<Leap::Bone::Type>( 0 ) ).prevJoint();

            joint->setMatrix( osg::Matrix::translate( this->center[0] +this->direction[0]+ static_cast<double>( posVector.x )/100.0,
                              this->center[1] +this->direction[1]*1+ static_cast<double>( -posVector.z )/100.0,
                              this->center[2] +this->direction[2]+ static_cast<double>( posVector.y )/100.0 ) );
        }
    }
    // vykreslenie klbov prstov
    unsigned int i = 0;
    for ( i= 0; i < 4; i++ ) {
        Leap::Joint* joint = static_cast<Leap::Joint*>( fingerJointGroup->getChild( i ) );

        Leap::Vector posVector = Leap::Vector( 0.0f,0.0f,0.0f );
        if ( fingerLeap.bone( static_cast<Leap::Bone::Type>( i ) ).isValid() ) {
            posVector = fingerLeap.bone( static_cast<Leap::Bone::Type>( i ) ).nextJoint();

            joint->setMatrix( osg::Matrix::translate(this->center[0] +this->direction[0]+ static_cast<double>( posVector.x )/100.0,
                              this->center[1] +this->direction[1]*1+ static_cast<double>( -posVector.z )/100.0,
                              this->center[2]+this->direction[2] + static_cast<double>( posVector.y )/100.0 ) );
        }

    }
}

void Leap::CustomLeapManager::updateFingerBones( osg::Group*  fingerBoneGroup, Leap::Finger fingerLeap, int fingerPosition )
{
    unsigned int i = 0;
    // ak ma prst 3 kosti (middle a  ring), tak je treba pouzit offset na data z leapu (leap 4 kosti, my 3)
    unsigned int offset = 0;
    if ( fingerPosition ==  2 || fingerPosition == 3 ) {
        offset = 1;
    }

    for ( i = 0; i < fingerBoneGroup->getNumChildren(); i++ ) {

        Leap::Vector posVector = Leap::Vector( 0.0f,0.0f,0.0f );
        Leap::Vector dirVector = Leap::Vector( 0.0f,0.0f,0.0f );


        if ( fingerLeap.bone( static_cast<Leap::Bone::Type>( i + offset ) ).isValid() ) {
            float length;
            Leap::HandBone* bone = static_cast<Leap::HandBone*>( fingerBoneGroup->getChild( i ) );
            // ziskanie dat z Leap senzoru
            posVector = fingerLeap.bone( static_cast<Leap::Bone::Type>( i + offset ) ).center();
            dirVector = fingerLeap.bone( static_cast<Leap::Bone::Type>( i + offset ) ).direction();
            length = fingerLeap.bone( static_cast<Leap::Bone::Type>( i + offset ) ).length();

            osg::RefMatrixd* boneMatrix = new osg::RefMatrixd();
            boneMatrix->makeIdentity();

            // position of bone
            boneMatrix->preMult( osg::Matrix::translate(this->center[0] +this->direction[0]+ static_cast<double>( posVector.x )/100.0,
                                 this->center[1] +this->direction[1]*1+ static_cast<double>( -posVector.z )/100.0,
                                 this->center[2] +this->direction[2] +static_cast<double>( posVector.y )/100.0 ) );

            // rotation of bone
            if ( dirVector.x != 0 || dirVector.y !=0 || dirVector.z !=0 ) {
                boneMatrix->preMult( osg::Matrix::rotate( osg::Vec3f( 0.0f,0.0f,1.0f ) ,
                                     osg::Vec3f( static_cast<osg::Vec3f::value_type>( dirVector.x/100.0f ),
                                                 static_cast<osg::Vec3f::value_type>( -( dirVector.z/100.0f ) ) ,
                                                 static_cast<osg::Vec3f::value_type>( dirVector.y/100.0f ) ) ) );
                boneMatrix->preMult( osg::Matrix::rotate(
                                     osg::Vec3f( static_cast<osg::Vec3f::value_type>( dirVector.x/100.0f ),
                                                 static_cast<osg::Vec3f::value_type>( -( dirVector.z/100.0f ) ) ,
                                                 static_cast<osg::Vec3f::value_type>( dirVector.y/100.0f ) ),
                                     osg::Vec3f( this->direction[0],this->direction[1],this->direction[2] )
                                         ) );
            }
            // scaling of bone
            boneMatrix->preMult( osg::Matrix::scale( 1.0,1.0,( static_cast<double>( length )/100.0 )/static_cast<double>( bone->HEIGHT ) ) );

            bone->setMatrix( *boneMatrix );
        }

    }
}

void Leap::CustomLeapManager::updateInterFingerBones( osg::Group*  interFingerBoneGroup, Leap::FingerList fingers )
{
    int i;
    Leap::Vector arrayOfJoints [4];

    // update kosti v zapasti
    this->updateInterFingerWristBone( interFingerBoneGroup, fingers );

    // inicializuju sa pozocie klbov medzi prstami
    for ( i = 1; i < 5; i++ ) {
        arrayOfJoints[i-1] = fingers[i].bone( static_cast<Leap::Bone::Type>( 0 ) ).nextJoint();
    }

    // ziskanie pozicii kosti medzi prstami
    Leap::Vector arrayOfInterFingerBonesPositions [3];
    for ( i = 0; i < 3; i++ ) {
        arrayOfInterFingerBonesPositions[i] = ( arrayOfJoints[i] + arrayOfJoints[i+1] ) / 2;
    }

    // ziskanie smeru kosti medzi prstami
    Leap::Vector arrayOfInterFingerBonesRotations [3];
    for ( i = 0; i < 3; i++ ) {
        arrayOfInterFingerBonesRotations[i] = ( arrayOfJoints[i+1] - arrayOfJoints[i] );
    }

    // ziskanie dlzky kosti medzi prstami
    float arrayOfInterFingerBonesLengths [3];
    for ( i = 0; i < 3; i++ ) {
        arrayOfInterFingerBonesLengths[i] = sqrtf( powf( ( arrayOfJoints[i+1].x - arrayOfJoints[i].x ),2 ) + powf( ( arrayOfJoints[i+1].y - arrayOfJoints[i].y ),2 )+ powf( ( arrayOfJoints[i+1].z - arrayOfJoints[i].z ),2 ) );

    }

    for ( i = 0; i < 3; i++ ) {
        Leap::HandBone* bone = static_cast<Leap::HandBone*>( interFingerBoneGroup->getChild( static_cast<unsigned int>( i ) ) );

        osg::RefMatrixd* boneMatrix = new osg::RefMatrixd();
        boneMatrix->makeIdentity();

        // position of bone
        boneMatrix->preMult( osg::Matrix::translate(this->center[0]+this->direction[0]+ static_cast<double>( arrayOfInterFingerBonesPositions[i].x )/100.0,
                             this->center[1] +this->direction[1]*1 +static_cast<double>( - arrayOfInterFingerBonesPositions[i].z )/100.0,
                             this->center[2] +this->direction[2]+ static_cast<double>( arrayOfInterFingerBonesPositions[i].y )/100.0 ) );

        // rotation of bone
        if ( arrayOfInterFingerBonesRotations[i].x != 0 || arrayOfInterFingerBonesRotations[i].y !=0 || arrayOfInterFingerBonesRotations[i].z !=0 ) {
            boneMatrix->preMult( osg::Matrix::rotate( osg::Vec3f( 0.0f,0.0f,1.0f ) ,
                                 osg::Vec3f( arrayOfInterFingerBonesRotations[i].x/100.0f ,
                                             -( arrayOfInterFingerBonesRotations[i].z/100.0f ) ,
                                             arrayOfInterFingerBonesRotations[i].y/100.0f ) ) );
        }
        // scaling of bone
        boneMatrix->preMult( osg::Matrix::scale( 1.0,1.0,( static_cast<double>( arrayOfInterFingerBonesLengths[i] )/100.0 )/static_cast<double>( bone->HEIGHT ) ) );

        bone->setMatrix( *boneMatrix );

    }
}

void Leap::CustomLeapManager::updateInterFingerWristBone( osg::Group*  interFingerBoneGroup, Leap::FingerList fingers )
{
    Leap::Vector positionOfInnerJoint;
    Leap::Vector positionOfOuterJoint;
    Leap::Vector bonePosition;
    Leap::Vector boneDirection;
    float boneLength;

    // 4-ta kost v interFingerBoneGroup
    Leap::HandBone* bone = static_cast<Leap::HandBone*>( interFingerBoneGroup->getChild( 3 ) );

    //ukazovak, najspodnejsia kost
    positionOfInnerJoint = fingers[1].bone( static_cast<Leap::Bone::Type>( 0 ) ).prevJoint();
    //malicek, najspodnejsia kost
    positionOfOuterJoint = fingers[4].bone( static_cast<Leap::Bone::Type>( 0 ) ).prevJoint();

    bonePosition = ( positionOfInnerJoint + positionOfOuterJoint ) / 2;

    boneDirection = ( positionOfInnerJoint - positionOfOuterJoint );

    boneLength = sqrtf( powf( ( positionOfInnerJoint.x - positionOfOuterJoint.x ),2 )
                        + powf( ( positionOfInnerJoint.y - positionOfOuterJoint.y ),2 )
                        + powf( ( positionOfInnerJoint.z - positionOfOuterJoint.z ),2 ) );
    boneLength = ( boneLength/100.0f ) /bone->HEIGHT;

    osg::RefMatrixd* boneMatrix = new osg::RefMatrixd();
    boneMatrix->makeIdentity();

    // position of bone
    boneMatrix->preMult( osg::Matrix::translate( this->center[0]+this->direction[0] + static_cast<double>( bonePosition.x )/100.0,
                         this->center[1] +this->direction[1]*1 +static_cast<double>( - bonePosition.z )/100.0,
                         this->center[2]+this->direction[2] +static_cast<double>( bonePosition.y )/100.0 ) );

    // rotation of bone
    if ( boneDirection.x != 0 || boneDirection.y !=0 || boneDirection.z !=0 ) {
        boneMatrix->preMult( osg::Matrix::rotate( osg::Vec3f( 0.0f,0.0f,1.0f ) ,
                             osg::Vec3f( boneDirection.x/100.0f ,
                                         -( boneDirection.z/100.0f ) ,
                                         boneDirection.y/100.0f ) ) );
    }
    // scaling of bone
    boneMatrix->preMult( osg::Matrix::scale( 1.0,1.0,static_cast<double>( boneLength ) ) );

    bone->setMatrix( *boneMatrix );

}
