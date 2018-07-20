// Copyright (C) 2009, 2010 by Florent Lamiraux, Thomas Moulard, JRL.
//
// This file is part of the hpp-affordance-corba.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// See the COPYING file for more information.

#include <iostream>

#include <hpp/util/debug.hh>

#include "hpp/affordance/affordance-extraction.hh"
#include "hpp/affordance/operations.hh"
#include <hpp/model/collision-object.hh>
#include "affordance.impl.hh"

namespace hpp
{
namespace affordanceCorba
{
namespace impl
{
Afford::Afford () {}

Afford::Afford (const core::ProblemSolverPtr_t& /*problemSolver*/) {}

void Afford::setProblemSolverMap
(hpp::corbaServer::ProblemSolverMapPtr_t psMap)
{
    psMap_ = psMap;
    resetAffordanceConfig ();
}

void Afford::resetAffordanceConfig() throw (hpp::Error)
{
    problemSolver()->add
            <core::AffordanceConfig_t> ("Support", vector3_t (0.3,0.3,0.05));
    problemSolver()->add
            <core::AffordanceConfig_t> ("Lean", vector3_t (0.1,0.3,0.05));
}

affordance::OperationBases_t Afford::createOperations () throw (hpp::Error)
{
    if (!problemSolver()->has
            <core::AffordanceConfig_t> ("Support")) {
        throw hpp::Error ("No 'Support' affordance type found Afford::createOperations ()");
    }
    const model::vector3_t & sconf = problemSolver()->get
            <core::AffordanceConfig_t> ("Support");

    if (!problemSolver()->has
            <core::AffordanceConfig_t> ("Lean")) {
        throw hpp::Error ("No 'Lean' affordance type found in Afford::createOperations ()");
    }
    const model::vector3_t & lconf = problemSolver()->get
            <core::AffordanceConfig_t> ("Lean");

    affordance::SupportOperationPtr_t support
            (new affordance::SupportOperation(sconf[0], sconf[1], sconf[2]));
    affordance::LeanOperationPtr_t lean
            (new affordance::LeanOperation(lconf[0], lconf[1], lconf[2]));

    affordance::OperationBases_t operations;
    operations.push_back(support);
    operations.push_back(lean);

    return operations;
}

void Afford::setAffordanceConfig (const char* affType,
                                  const hpp::doubleSeq& conf) throw (hpp::Error)
{
    if (conf.length () != 3) {
        throw hpp::Error ("Configuration vector has invalid size.");
    }
    const vector3_t config (conf[0], conf[1], conf[2]);
    problemSolver()->add
            <core::AffordanceConfig_t> (affType, config);

    const std::map<std::string, core::AffordanceConfig_t> map = problemSolver()->map
            <core::AffordanceConfig_t> ();
}
hpp::doubleSeq* Afford::getAffordanceConfig (const char* affType)
throw (hpp::Error)
{
    if (!problemSolver()->has
            <core::AffordanceConfig_t> (affType)) {
        throw hpp::Error ("No given affordance type found in Afford::getAffordanceConfig");
    }
    const vector3_t& config = problemSolver()->get
            <core::AffordanceConfig_t> (affType);
    hpp::doubleSeq* conf = new hpp::doubleSeq ();
    conf->length ((CORBA::ULong)config.size ());
    for (std::size_t idx = 0; idx < conf->length(); idx++) {
        (*conf)[(CORBA::ULong)idx] = config[idx];
    }
    return conf;
}

void Afford::setMargin (const char* affType, CORBA::Double margin)
throw (hpp::Error)
{
    if (!problemSolver()->has
            <core::AffordanceConfig_t> (affType)) {
        throw hpp::Error ("No given affordance type found in Afford::setMargin");
    }
    vector3_t config = problemSolver()->get
            <core::AffordanceConfig_t> (affType);
    config[0] = margin;

    problemSolver()->add
            <core::AffordanceConfig_t> (affType, config);
}

void Afford::setNeighbouringTriangleMargin (const char* affType,
                                            CORBA::Double nbTriMargin) throw (hpp::Error)
{
    if (!problemSolver()->has
            <core::AffordanceConfig_t> (affType)) {
        throw hpp::Error ("No given affordance type found in Afford::setNeighbouringTriangleMargin");
    }
    vector3_t config = problemSolver()->get
            <core::AffordanceConfig_t> (affType);
    config[1] = nbTriMargin;

    problemSolver()->add
            <core::AffordanceConfig_t> (affType, config);
}

void Afford::setMinimumArea (const char* affType, CORBA::Double minArea)
throw (hpp::Error)
{
    if (!problemSolver()->has
            <core::AffordanceConfig_t> (affType)) {
        throw hpp::Error ("No given affordance type found in Afford::setMinimunArea");
    }
    vector3_t config = problemSolver()->get
            <core::AffordanceConfig_t> (affType);
    config[2] = minArea;

    problemSolver()->add
            <core::AffordanceConfig_t> (affType, config);
}

bool isBVHModelTriangles (const fcl::CollisionObjectPtr_t& object)
{
    if (object->collisionGeometry ()->getNodeType () == fcl::BV_OBBRSS) {
        const affordance::BVHModelOBConst_Ptr_t model = boost::static_pointer_cast<const affordance::BVHModelOB>
                (object->collisionGeometry ());
        if (model->getModelType () == fcl::BVH_MODEL_TRIANGLES) {
            return true;
        }
    }
    return false;
}

bool Afford::checkModel (const char* obstacleName) throw (hpp::Error)
{
    std::list<std::string> obstacles =
            problemSolver()->obstacleNames(false, true);
    std::list<std::string>::iterator objIt = std::find
            (obstacles.begin (), obstacles.end (), obstacleName);
    if (objIt == obstacles.end ()) {
        throw hpp::Error ("No obstacle by given name found. Unable to analyse.");
    }
    if (!isBVHModelTriangles ((problemSolver()->obstacle (obstacleName))->fcl ())){
        return false; // wrong model type -> return false
    }
    return true;
}

void Afford::affordanceAnalysis (const char* obstacleName,
                                 const affordance::OperationBases_t & operations) throw (hpp::Error)
{
    std::list<std::string> obstacles =
            problemSolver()->obstacleNames(true, true);
    std::list<std::string>::iterator objIt = std::find
            (obstacles.begin (), obstacles.end (), obstacleName);
    if (objIt == obstacles.end ()) {
        throw hpp::Error ("No obstacle by given name found. Unable to analyse.");
    }
    try {
        affordance::SemanticsDataPtr_t aff = affordance::affordanceAnalysis
                ((problemSolver()->obstacle (obstacleName))->fcl (), operations);
        std::vector<std::vector<fcl::CollisionObjectPtr_t > > affObjs =
                affordance::getAffordanceObjects (aff);
        // add fcl::CollisionObstacles to problemSolver
        addAffObjects (operations, affObjs, obstacleName);
    } catch (const std::exception& exc) {
        throw Error (exc.what ());
    }
}

void Afford::analyseObject (const char* obstacleName) throw (hpp::Error)
{
    // first erase affordance information for obstacleName
    deleteAffordances(obstacleName);
    affordance::OperationBases_t operations = createOperations ();
    affordanceAnalysis (obstacleName, operations);
}

void Afford::analyseAll () throw (hpp::Error)
{
    // first clear all old affordances:
    problemSolver()->clear <std::vector<boost::shared_ptr<model::CollisionObject> > > ();
    affordance::OperationBases_t operations = createOperations ();
    for (std::list <boost::shared_ptr<model::CollisionObject> >::const_iterator objIt =
         problemSolver()->collisionObstacles ().begin ();
         objIt != problemSolver()->collisionObstacles ().end (); objIt++)
    {
        const char* obstacleName = (*objIt)->name ().c_str ();
        affordanceAnalysis (obstacleName, operations);
    }
}

// delete affordances by type for given object
void Afford::deleteAffordancesByType (const char* affordance,
                                      const char* obstacleName) throw (hpp::Error)
{
    const std::string noObject = "";
    if (obstacleName == noObject) {
        problemSolver()->erase
                <std::vector<boost::shared_ptr<model::CollisionObject> > > (affordance);
    } else {
        if (!problemSolver()->has <std::vector<boost::shared_ptr<model::CollisionObject> > > (affordance)) {
            std::cout << "Afford::deleteAffordanceByType: no affordance objects to delete" << std::endl;
            return;
        }
        std::vector<boost::shared_ptr<model::CollisionObject> > affs =
                problemSolver()->get
                <std::vector<boost::shared_ptr<model::CollisionObject> > > (affordance);

        for (unsigned int objIdx = 0; objIdx < affs.size (); objIdx++)
        {
            if (affs[objIdx]->name () == obstacleName) {
                affs.erase(affs.begin () + objIdx);
                objIdx--;
            }
        }
        problemSolver()->add
                <std::vector<boost::shared_ptr<model::CollisionObject> > >
                (affordance,affs);
    }
}

// delete all affordances for given object
void Afford::deleteAffordances (const char* obstacleName)
throw (hpp::Error)
{
    const std::string noObject = "";
    if (obstacleName == noObject) {
        // if no obstacleName given, delete all affs in problemSolver
        problemSolver()->clear <std::vector<boost::shared_ptr<model::CollisionObject> > > ();
    } else {
        std::list<std::string> keys =  problemSolver()->getKeys
                <std::vector<boost::shared_ptr<model::CollisionObject> >,
                std::list<std::string> > ();
        std::list<std::string>::iterator affIt = keys.begin ();
        for (; affIt != keys.end (); affIt++) {
            std::vector<boost::shared_ptr<model::CollisionObject> > affs =
                    problemSolver()->get
                    <std::vector<boost::shared_ptr<model::CollisionObject> > > (*affIt);

            for (unsigned int objIdx = 0; objIdx < affs.size (); objIdx++)
            {
                if (affs[objIdx]->name () == obstacleName) {
                    affs.erase(affs.begin () + objIdx);
                    objIdx--;
                }
            }
            problemSolver()->add
                    <std::vector<boost::shared_ptr<model::CollisionObject> > >
                    (*affIt,affs);
        }
    }
}


void Afford::addAffObjects (const affordance::OperationBases_t& ops,
                            const std::vector<affordance::CollisionObjects_t>& affObjs,
                            const char* obstacleName) throw (hpp::Error)
{
    for (unsigned int opIdx = 0; opIdx < ops.size (); opIdx++)
    {
        std::vector<boost::shared_ptr<model::CollisionObject> > objs;
        affordance::CollisionObjects_t affs = affObjs[opIdx];
        for (unsigned int objIdx = 0; objIdx < affs.size (); objIdx++) {
            boost::shared_ptr<model::CollisionObject> obj =
                    model::CollisionObject::create (affs[objIdx], obstacleName);
            objs.push_back (obj);
        }
        if (problemSolver()->has
                <std::vector<boost::shared_ptr<model::CollisionObject> > >
                (ops[opIdx]->affordance_)) {
            std::vector<boost::shared_ptr<model::CollisionObject> >
                    mapObjs = problemSolver()->get
                    <std::vector<boost::shared_ptr<model::CollisionObject> > >
                    (ops[opIdx]->affordance_);
            objs.insert (objs.begin () + objs.size (), mapObjs.begin (), mapObjs.end ());
        }
        problemSolver()->add
                <std::vector<boost::shared_ptr<model::CollisionObject> > >(ops[opIdx]->affordance_, objs);
    }
}

hpp::doubleSeqSeqSeqSeq* Afford::getAffordancePoints (char const* affordance)
throw (hpp::Error)
{
    hpp::doubleSeqSeqSeqSeq *affs;
    if (!problemSolver()->has
            <std::vector<boost::shared_ptr<model::CollisionObject> > >
            (std::string (affordance))) {
        throw hpp::Error ("No affordance type of given name found. Unable to get affordance points.");
    }
    std::vector<boost::shared_ptr<model::CollisionObject> > affObjs =
            problemSolver()->get
            <std::vector<boost::shared_ptr<model::CollisionObject> > >
            (affordance);
    std::size_t nbAffs = affObjs.size ();
    affs = new hpp::doubleSeqSeqSeqSeq ();
    affs->length ((CORBA::ULong)nbAffs);
    for (std::size_t affIdx = 0; affIdx < nbAffs; affIdx++)
    {
        affordance::BVHModelOBConst_Ptr_t model =
                affordance::GetModel (affObjs[affIdx]->fcl());
        std::size_t nbTris = model->num_tris;
        hpp::doubleSeqSeqSeq tris;
        tris.length ((CORBA::ULong)nbTris);
        for (std::size_t triIdx = 0; triIdx < nbTris; triIdx++)
        {
            hpp::doubleSeqSeq triangle;
            const fcl::Triangle& refTri = model->tri_indices[triIdx];
            triangle.length (3);
            for (unsigned int vertIdx= 0; vertIdx < 3; vertIdx++) {
                fcl::Vec3f p (affObjs[affIdx]->fcl ()->getRotation () *
                              model->vertices [refTri[vertIdx]] +
                        affObjs[affIdx]->fcl ()->getTranslation ());
                hpp::doubleSeq point;
                // point always 3D
                point.length (3);
                for (std::size_t idx = 0; idx < 3; idx++) {
                    point[(CORBA::ULong)idx] = p[idx];
                }
                triangle[(CORBA::ULong)vertIdx] = point;
            }
            tris[(CORBA::ULong)triIdx] = triangle;
        }
        (*affs)[(CORBA::ULong)affIdx] = tris;
    }
    return affs;
}

hpp::Names_t* fromStringVector(const std::vector<std::string>& input)
{
    CORBA::ULong size = (CORBA::ULong)input.size ();
    char** nameList = hpp::Names_t::allocbuf(size);
    hpp::Names_t *jointNames = new hpp::Names_t (size, size, nameList);
    for (std::size_t i = 0; i < input.size (); ++i)
    {
        std::string name = input[i];
        nameList [i] =
                (char*) malloc (sizeof(char)*(name.length ()+1));
        strcpy (nameList [i], name.c_str ());
    }
    return jointNames;
}

hpp::Names_t* Afford::getAffRefObstacles (const char* affordance)
throw (hpp::Error)
{
    std::vector<std::string> objList;
    if (!problemSolver()->has
            <std::vector<boost::shared_ptr<model::CollisionObject> > >
            (std::string (affordance))) {
        throw hpp::Error ("No affordance type of given name found. Unable to get reference collision object.");
    }
    std::vector<boost::shared_ptr<model::CollisionObject> > affObjs =
            problemSolver()->get
            <std::vector<boost::shared_ptr<model::CollisionObject> > >
            (affordance);
    for (std::size_t affIdx = 0; affIdx < affObjs.size (); affIdx++)
    {
        affordance::BVHModelOBConst_Ptr_t model =
                affordance::GetModel (affObjs[affIdx]->fcl());
        objList.push_back(affObjs[affIdx]->name ());
    }
    hpp::Names_t* objListPtr = fromStringVector (objList);
    return objListPtr;
}

hpp::Names_t* Afford::getAffordanceTypes () throw (hpp::Error)
{
    const std::map<std::string, std::vector<boost::shared_ptr<
            model::CollisionObject> > >& affMap = problemSolver()->map
            <std::vector<boost::shared_ptr<model::CollisionObject> > > ();
    if (affMap.empty ()) {
        throw hpp::Error ("No affordances found. Return empty list.");
    }
    std::vector<std::string> affTypes = problemSolver()->getKeys
            <std::vector<boost::shared_ptr<model::CollisionObject> >,
            std::vector<std::string> > ();
    hpp::Names_t* affTypeListPtr = fromStringVector (affTypes);
    return affTypeListPtr;
}

hpp::Names_t* Afford::getAffordanceConfigTypes () throw (hpp::Error)
{
    const std::map<std::string, core::AffordanceConfig_t>& affMap = problemSolver()->map
            <core::AffordanceConfig_t> ();
    if (affMap.empty ()) {
        std::cout << "No affordance configs found. Return empty list." << std::endl;
        hpp::Names_t* empty = new hpp::Names_t ();
        empty->length (0);
        return empty;
    }
    std::vector<std::string> affTypes = problemSolver()->getKeys
            <core::AffordanceConfig_t,
            std::vector<std::string> > ();
    hpp::Names_t* affTypeListPtr = fromStringVector (affTypes);
    return affTypeListPtr;
}


} // namespace impl
} // namespace affordanceCorba
} // namespace hpp
