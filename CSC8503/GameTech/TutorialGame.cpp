#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/Constraint.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../GameTech/StateGameObject.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
    world = new GameWorld();
    renderer = new GameTechRenderer(*world);
    physics = new PhysicsSystem(*world);

    forceMagnitude = 10.0f;
    useGravity = false;
    inSelectionMode = false;

    Debug::SetRenderer(renderer);

    testStateObject = nullptr;
    InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
    auto loadFunc = [](const string& name, OGLMesh** into) {
        *into = new OGLMesh(name);
        (*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
        (*into)->UploadToGPU();
    };

    loadFunc("cube.msh", &cubeMesh);
    loadFunc("sphere.msh", &sphereMesh);
    loadFunc("Male1.msh", &charMeshA);
    loadFunc("courier.msh", &charMeshB);
    loadFunc("security.msh", &enemyMesh);
    loadFunc("coin.msh", &bonusMesh);
    loadFunc("capsule.msh", &capsuleMesh);

    basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
    basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

TutorialGame::~TutorialGame() {
    delete cubeMesh;
    delete sphereMesh;
    delete charMeshA;
    delete charMeshB;
    delete enemyMesh;
    delete bonusMesh;

    delete basicTex;
    delete basicShader;

    delete physics;
    delete renderer;
    delete world;
}

void TutorialGame::ResetWorld()
{
    world = new GameWorld();
    renderer = new GameTechRenderer(*world);
    physics = new PhysicsSystem(*world);


    forceMagnitude = 10.0f;
    useGravity = false;
    inSelectionMode = false;

    Debug::SetRenderer(renderer);

    testStateObject = nullptr;
    InitialiseAssets();
}

//Main update function
void TutorialGame::UpdateGame(float dt) {
    if (!inSelectionMode) {
        world->GetMainCamera()->UpdateCamera(dt);
    }

    UpdateKeys();

    if (useGravity) {
        Debug::Print("(G)ravity on", Vector2(5, 95));
    }
    else {
        Debug::Print("(G)ravity off", Vector2(5, 95));
    }

    SelectObject();
    MoveSelectedObject();
    physics->Update(dt);

    if (lockedObject != nullptr) {
        Vector3 objPos = lockedObject->GetTransform().GetPosition();
        Vector3 camPos = objPos + lockedOffset;

        Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

        Matrix4 modelMat = temp.Inverse();

        Quaternion q(modelMat);
        Vector3 angles = q.ToEuler(); //nearly there now!

        world->GetMainCamera()->SetPosition(camPos);
        world->GetMainCamera()->SetPitch(angles.x);
        world->GetMainCamera()->SetYaw(angles.y);

        //Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
    }

    world->UpdateWorld(dt);
    renderer->Update(dt);

    Debug::FlushRenderables(dt);
    renderer->Render();

    // Game logic
    if (isInLevel1)
        GameLogicLevel1(dt);
    else if (isInLevel2)
        GameLogicLevel2(dt);
}

//Game Level1
void TutorialGame::InitGameBoardLevel1()
{
    InitCameraLevel1();
    InitWorldLevel1();
}

void TutorialGame::InitCameraLevel1() {
    world->GetMainCamera()->SetNearPlane(0.1f);
    world->GetMainCamera()->SetFarPlane(500.0f);
    world->GetMainCamera()->SetPitch(-15.0f);
    world->GetMainCamera()->SetYaw(315.0f);
    world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
    lockedObject = nullptr;
}

void TutorialGame::InitWorldLevel1() {
    world->ClearAndErase();
    physics->Clear();

    //InitMixedGridWorld(5, 5, 3.5f, 3.5f);

    InitDefaultFloorAndWall();
    InitGameElementsLevel1();

    //BridgeConstraintTest();
}

void TutorialGame::GameLogicLevel1(float dt)
{
    if (!isLevel1End)
    {
        spentTime += dt;
        renderer->DrawString("Spent Time: " + std::to_string(spentTime), Vector2(5, 15));
    }
    else
        OnGameEnd();
    if (testStateObject)
        testStateObject->Update(dt);

    //Judge for game ending;
    const auto collisionsInfo = physics->GetAllCollisionsInfos();
    for (auto& info : collisionsInfo)
    {
        if ((info.a == endGameInfo.a && info.b == endGameInfo.b) || (info.a == endGameInfo.b && info.b == endGameInfo.a))
        {
            isLevel1End = true;
            scoreLevel1 = 1000 * (1 - spentTime / 120);
            scoreLevel1 = scoreLevel1 > 0 ? scoreLevel1 : 0;
        }
    }
}

//Game Level2

void TutorialGame::InitGameBoardLevel2()
{
    InitCameraLevel2();
    InitWorldLevel2();
}

void TutorialGame::InitWorldLevel2() {
    world->ClearAndErase();
    physics->Clear();

    InitDefaultFloorAndWallLevel2();
    InitGameElementsLevel2();

    PfManager = new PathfindingManager();
    lastEnemyPos = Vector3(180, 5, 10);
    lastPlayerPos = Vector3(180, 0, 180);
    PfManager->TestPathfinding(lastEnemyPos, lastPlayerPos);
    const auto pathNodes = PfManager->GetPathNodes();
    currentPfIndex = pathNodes.size() - 1;
    nextPfIndex = currentPfIndex - 1;
}

void TutorialGame::InitCameraLevel2() {
    world->GetMainCamera()->SetNearPlane(0.1f);
    world->GetMainCamera()->SetFarPlane(500.0f);
    world->GetMainCamera()->SetPitch(-90.0f);
    world->GetMainCamera()->SetYaw(0.0f);
    world->GetMainCamera()->SetPosition(Vector3(100, 250, 100));
    lockedObject = nullptr;
}

void TutorialGame::GameLogicLevel2(float dt)
{
    //if ((playerBall->GetTransform().GetPosition() - lastPlayerPos).Length() > 1)
    //{
    //    PfManager->ClearPathNodes();
    //    lastPlayerPos = playerBall->GetTransform().GetPosition();
    //    lastEnemyPos = enemyBall->GetTransform().GetPosition();
    //    PfManager->TestPathfinding(lastEnemyPos, lastPlayerPos);
    //    nextPfIndex = PfManager->GetPathNodes().size() - 2;
    //}

    const auto pathNodes = PfManager->GetPathNodes();
    PfManager->DisplayPathfinding();
    AiBehaviour(dt, pathNodes);

    AddRayToEnemyToWorld(enemyBall);
    StateAI(enemyBall,bonus1,enemySeeBonus);

    //Judge for game ending and bonus collection;;
    Debug::Print("Score: " + std::to_string(scoreLevel2), Vector2(5, 25));
    const auto collisionsInfo = physics->GetAllCollisionsInfos();
    for (auto& info : collisionsInfo)
    {
        if (info.a == endGameInfo.a && info.b == endGameInfo.b || 
            info.a == endGameInfo.b && info.b == endGameInfo.a)
        {
            InitWorldLevel2();
            currentPfIndex = pathNodes.size() - 1;
            nextPfIndex = currentPfIndex - 1;
        }
        if (info.a->GetWorldID() == 100 && info.b->GetWorldID() == 202 ||
            info.a->GetWorldID() == 202 && info.b->GetWorldID() == 100 ||
            info.a->GetWorldID() == 100 && info.b->GetWorldID() == 203 ||
            info.a->GetWorldID() == 203 && info.b->GetWorldID() == 100)
        {
            scoreLevel2 += 100;
            if (info.a->GetWorldID() == 202 || info.b->GetWorldID() == 202)
                bonus1->GetTransform().SetPosition(Vector3(150, -20, 10));
            if (info.a->GetWorldID() == 203 || info.b->GetWorldID() == 203)
                bonus2->GetTransform().SetPosition(Vector3(130, -20, 10));
        }
    }
}

void TutorialGame::InitDefaultFloorAndWallLevel2() {
    AddFloorToWorld(Vector3(100, -7, 100));
    AddWallToWorld(Vector3(202, 10, 100), Vector3(2, 10, 104));
    AddWallToWorld(Vector3(-2, 10, 100), Vector3(2, 10, 104));
    AddWallToWorld(Vector3(100, 10, -2), Vector3(100, 10, 2));
    AddWallToWorld(Vector3(100, 10, 202), Vector3(100, 10, 2));

    //Maze wall
    AddWallToWorld(Vector3(20, 15, 30), Vector3(20, 15, 5));
    AddWallToWorld(Vector3(160, 15, 70), Vector3(40, 15, 5));
    AddWallToWorld(Vector3(100, 15, 120), Vector3(40, 15, 5));
    AddWallToWorld(Vector3(140, 15, 170), Vector3(60, 15, 5));
}

//Initialise game elements to the world Level2
void TutorialGame::InitGameElementsLevel2()
{
    InitTargetBall(Vector3(180, 0, 10));
    InitTargetEnemyBall(Vector3(180, 0, 180));
    InitBonus();
}

void TutorialGame::OnGameEnd()
{
    if (isLevel1End)
    {
        Debug::Print("Congratulate! Total Spent Time: " + std::to_string(spentTime), Vector2(5, 35));
        Debug::Print("Your score: " + std::to_string(scoreLevel1), Vector2(5, 45));
    }
}

void TutorialGame::AiBehaviour(float dt, const vector<Vector3>& pathNodes)
{
    //Print Emery Position
    const auto emeryBallPosition = enemyBall->GetTransform().GetPosition();

    //Move forward
    const auto nextNode = pathNodes[nextPfIndex];
    const auto tempLength = (nextNode - emeryBallPosition).Length();
    const auto curForce = (nextNode - emeryBallPosition).Normalised() * Vector3(10.0f, 10.0f, 10.0f);

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
    {
        Debug::Print("Current force: " +
        std::to_string(curForce.x) + " " +
        std::to_string(curForce.y) + " " +
        std::to_string(curForce.z),
        Vector2(25, 15));

        Debug::Print(
        "Emery Ball Position: " +
        std::to_string(static_cast<int>(emeryBallPosition.x)) + " " +
        std::to_string(static_cast<int>(emeryBallPosition.y)) + " " +
        std::to_string(static_cast<int>(emeryBallPosition.z)),
        Vector2(25, 5));
}

    if (tempLength < 2)
    {
        currentPfIndex--;
        nextPfIndex--;
        if (currentPfIndex < 1)
            currentPfIndex = 1;
        if (nextPfIndex < 0)
            nextPfIndex = 0;
    }
    enemyBall->GetPhysicsObject()->AddForce(curForce);
}

void TutorialGame::AddRayToEnemyToWorld(StateGameObject* stateGameObject)
{
    //StateGameObject is Enemy!!!
    //Set Four Ray
    Vector3 rayStartPosition = stateGameObject->GetTransform().GetPosition();

    auto rayDirection1 = Vector3(400, 0, 0);
    auto rayDirection2 = Vector3(-400, 0, 0);
    auto rayDirection3 = Vector3(0, 0, 400);
    auto rayDirection4 = Vector3(0, 0, -400);

    auto ray1 = Ray(rayStartPosition + rayDirection1 * 3.0f,rayDirection1);
    auto ray2 = Ray(rayStartPosition + rayDirection2 * 3.0f,rayDirection2);
    auto ray3 = Ray(rayStartPosition + rayDirection3 * 3.0f,rayDirection3);
    auto ray4 = Ray(rayStartPosition + rayDirection4 * 3.0f,rayDirection4);

    Debug::DrawLine(ray1.GetPosition(),ray1.GetDirection());
    Debug::DrawLine(ray2.GetPosition(),ray2.GetDirection());
    Debug::DrawLine(ray3.GetPosition(),ray3.GetDirection());
    Debug::DrawLine(ray4.GetPosition(),ray4.GetDirection());

    RayCollision rayCollision1;
    RayCollision rayCollision2;
    RayCollision rayCollision3;
    RayCollision rayCollision4;

    world->Raycast(ray1, rayCollision1,true);
    world->Raycast(ray2, rayCollision2, true);
    world->Raycast(ray3, rayCollision3, true);
    world->Raycast(ray4, rayCollision4, true);

    auto* objectSaw1 = static_cast<GameObject*>(rayCollision1.node);
    auto* objectSaw2 = static_cast<GameObject*>(rayCollision1.node);
    auto* objectSaw3 = static_cast<GameObject*>(rayCollision1.node);
    auto objectSaw4 = static_cast<GameObject*>(rayCollision1.node);

    if ((objectSaw1 == bonus1 || objectSaw1 == bonus2 )
        ||(objectSaw2 == bonus1 || objectSaw2 == bonus2)
        || (objectSaw3 == bonus1 || objectSaw3 == bonus2)
        || (objectSaw4 == bonus1 || objectSaw4 == bonus2))
    {
        enemySeeBonus = true;
    }
    
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
    auto apple = new StateGameObject();
    float radius = 3.0f;
    auto volume = new SphereVolume(radius);
    apple->SetBoundingVolume(reinterpret_cast<CollisionVolume*>(volume));
    apple->GetTransform()
        .SetScale(Vector3(radius, radius, radius))
        .SetPosition(position);

    apple->SetRenderObject(new RenderObject(&apple->GetTransform(), sphereMesh, basicTex, basicShader));
    apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

    apple->GetPhysicsObject()->SetInverseMass(1.0f);
    apple->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(apple);

    return apple;
}

void TutorialGame::UpdateKeys() {
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F5)) {
        InitWorldLevel1(); //We can reset the simulation at any time with F5
        selectionObject = nullptr;
        lockedObject = nullptr;
        isLevel1End = false;
        spentTime = 0;
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
        InitCameraLevel1(); //F2 will reset the camera to a specific default place
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
        useGravity = !useGravity; //Toggle gravity!
        physics->UseGravity(useGravity);
    }
    //Running certain physics updates in a consistent order might cause some
    //bias in the calculations - the same objects might keep 'winning' the constraint
    //allowing the other one to stretch too much etc. Shuffling the order so that it
    //is random every frame can help reduce such bias.
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
        world->ShuffleConstraints(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
        world->ShuffleConstraints(false);
    }

    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
        world->ShuffleObjects(true);
    }
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
        world->ShuffleObjects(false);
    }

    if (lockedObject) {
        LockedObjectMovement();
    }
    else {
        DebugObjectMovement();
    }
}

void TutorialGame::LockedObjectMovement() {
    Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
    Matrix4 camWorld = view.Inverse();

    Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

    //forward is more tricky -  camera forward is 'into' the screen...
    //so we can take a guess, and use the cross of straight up, and
    //the right axis, to hopefully get a vector that's good enough!

    Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
    fwdAxis.y = 0.0f;
    fwdAxis.Normalise();

    Vector3 charForward = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
    Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

    float force = 1.0f;

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
        lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
        Vector3 worldPos = selectionObject->GetTransform().GetPosition();
        lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
        lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
        lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
    }

    if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
        lockedObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
    }
}

void TutorialGame::DebugObjectMovement() {
    //If we've selected an object, we can manipulate it with some key presses
    if (inSelectionMode && selectionObject) {
        //Twist the selected object!
        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
            selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
        }

        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
            selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
        }
        //if player is moving
        if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
        {
            auto pos = selectionObject->GetTransform().GetPosition();
            Debug::Print("Position: " +
                std::to_string(pos.x) + " " +
                std::to_string(pos.y) + " " +
                std::to_string(pos.z),
                Vector2(25, 10));
        }
    }

}

void TutorialGame::BridgeConstraintTest() {

    const auto cubeSize = Vector3(5, 5, 5);	// how heavy the middle pieces are
    const float invCubeMass = 5;
    constexpr int numLinks = 10;
    constexpr float maxDistance = 30;				// constraint distance
    constexpr float cubeDistance = 20;			// distance between links

    const auto startPos = Vector3(0, 100, 100);
    const auto start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
    const auto end = AddCubeToWorld(startPos + Vector3((numLinks + 3) * cubeDistance, 0, 0), cubeSize, 0);
    auto previous = start;
    for (int i = 0; i < numLinks; ++i)
    {
        const auto block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
        const auto constraint = new PositionConstraint(previous, block, maxDistance);
        world->AddConstraint(constraint);
        previous = block;
    }
    const auto constraint = new PositionConstraint(previous, end, maxDistance);
    world->AddConstraint(constraint);
}

/*
A single function to add three large immoveable cube around our world
*/
GameObject* TutorialGame::AddWallToWorld(const Vector3& position, const Vector3& size) const
{
    const auto wall = new GameObject();
    Vector3 wallSize = size;
    const auto volume = new AABBVolume(wallSize);
    wall->SetBoundingVolume(reinterpret_cast<CollisionVolume*>(volume));
    wall->GetTransform()
        .SetScale(wallSize * 2)
        .SetPosition(position);

    wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
    wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

    wall->GetPhysicsObject()->SetInverseMass(0);
    wall->GetPhysicsObject()->InitCubeInertia();
    wall->SetWorldID(1);
    world->AddGameObject(wall);

    return wall;
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) const
{
    GameObject* floor = new GameObject();

    Vector3 floorSize = Vector3(100, 2, 100);
    AABBVolume* volume = new AABBVolume(floorSize);
    floor->SetBoundingVolume((CollisionVolume*)volume);
    floor->GetTransform()
        .SetScale(floorSize * 2)
        .SetPosition(position);

    floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
    floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

    floor->GetPhysicsObject()->SetInverseMass(0);
    floor->GetPhysicsObject()->InitCubeInertia();
    floor->SetWorldID(2);
    world->AddGameObject(floor);

    return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
    const auto sphere = new GameObject();

    const auto sphereSize = Vector3(radius, radius, radius);
    const auto volume = new SphereVolume(radius);
    sphere->SetBoundingVolume(reinterpret_cast<CollisionVolume*>(volume));

    sphere->GetTransform()
        .SetScale(sphereSize)
        .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
    sphere->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(sphere);

    return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
    GameObject* capsule = new GameObject();

    CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
    capsule->SetBoundingVolume((CollisionVolume*)volume);

    capsule->GetTransform()
        .SetScale(Vector3(radius * 2, halfHeight, radius * 2))
        .SetPosition(position);

    capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
    capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

    capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
    capsule->GetPhysicsObject()->InitCubeInertia();

    world->AddGameObject(capsule);

    return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
    GameObject* cube = new GameObject();

    AABBVolume* volume = new AABBVolume(dimensions);

    cube->SetBoundingVolume((CollisionVolume*)volume);

    cube->GetTransform()
        .SetPosition(position)
        .SetScale(dimensions * 2);

    cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
    cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

    cube->GetPhysicsObject()->SetInverseMass(inverseMass);
    cube->GetPhysicsObject()->InitCubeInertia();

    world->AddGameObject(cube);

    return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddSphereToWorld(position, radius, 1.0f);
        }
    }
    AddFloorToWorld(Vector3(0, -2, 0));
}

//Initialise sample spheres and cubes
void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
    float sphereRadius = 1.0f;
    Vector3 cubeDims = Vector3(1, 1, 1);

    for (int x = 0; x < numCols; ++x) {
        for (int z = 0; z < numRows; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

            if (rand() % 2) {
                AddCubeToWorld(position, cubeDims);
            }
            else {
                AddSphereToWorld(position, sphereRadius);
            }
        }
    }
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
    for (int x = 1; x < numCols + 1; ++x) {
        for (int z = 1; z < numRows + 1; ++z) {
            Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
            AddCubeToWorld(position, cubeDims, 1.0f);
        }
    }
}

void TutorialGame::InitDefaultFloorAndWall() {
    AddFloorToWorld(Vector3(0, -2, 0));
    AddWallToWorld(Vector3(102, 15, 0), Vector3(2, 15, 104));
    AddWallToWorld(Vector3(-102, 15, 0), Vector3(2, 15, 104));
    AddWallToWorld(Vector3(0, 15, -102), Vector3(100, 15, 2));
    AddWallToWorld(Vector3(0, 15, 102), Vector3(100, 15, 2));
}
//Add the ball which will be sent to the end into the world
void TutorialGame::InitTargetBall(const Vector3& position)
{
    constexpr float sphereRadius = 3.0f;
    playerBall = AddSphereToWorld(position, sphereRadius, 100);
    playerBall->SetName("Ball");
    playerBall->SetWorldID(100);
    endGameInfo.a = playerBall;
}

void TutorialGame::InitTargetControllerCube(const Vector3& position)
{
    const auto cubeSize = Vector3(5, 5, 5);	// how heavy the middle pieces are
    const auto cube = AddCubeToWorld(position, cubeSize, 100);
    cube->SetName("TargetControllerCube");
    cube->SetWorldID(101);
}

void TutorialGame::InitTargetEnding(const Vector3& position)
{
    const auto cubeSize = Vector3(10, 10, 10);	// how heavy the middle pieces are
    const auto ending = AddCubeToWorld(position, cubeSize, 0);
    ending->SetName("Ending");
    ending->SetWorldID(102);
    endGameInfo.b = ending;
}

void TutorialGame::InitConstraintCubeAndRotatingSphere(const Vector3& cubePos, const Vector3& rsPos)
{
    const auto cubeSize = Vector3(5, 5, 5);
    const auto rsSize = Vector3(5, 5, 5);
    float invMassCube = 0;
    float invMassRS = 0.1;
    float loopLength = 20.0f;
    auto startPos = cubePos;
    auto rotatePos = rsPos;
    const auto cube = AddCubeToWorld(cubePos, cubeSize, invMassCube);
    const auto rsSphere = AddRotatingSphereToWorld(rsPos, 5, Window::GetTimer()->GetTimeDeltaSeconds(), 0);
    rsSphere->GetPhysicsObject()->SetInverseMass(invMassRS);
    rsSphere->GetPhysicsObject()->InitSphereInertia();
    rsSphere->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 10000, 0) * Window::GetTimer()->GetTimeDeltaSeconds());
    rsSphere->SetName("Rotating sphere");
    rsSphere->SetWorldID(104);

    auto constraint = new PositionConstraint(cube, rsSphere, loopLength);
    world->AddConstraint(constraint);
}

void TutorialGame::InitTargetEnemyBall(const Vector3& position)
{
    constexpr float sphereRadius = 3.0f;
    //enemyBall = AddSphereToWorld(position, sphereRadius, 100);
    enemyBall = AddStateObjectToWorld(position);
    enemyBall->SetName("Sphere Emery");
    enemyBall->SetWorldID(201);
    endGameInfo.b = enemyBall;
}

void TutorialGame::InitBonus()
{
    bonus1 = AddBonusToWorld(Vector3(150, 0, 55));
    bonus2 = AddBonusToWorld(Vector3(160, 0, 55));

    bonus1->SetWorldID(202);
    bonus2->SetWorldID(203);

    bonus1->SetName("Bonus1");
    bonus2->SetName("Bonus2");
}


GameObject* TutorialGame::AddRotatingSphereToWorld(const Vector3& position, int radius,
                                                   float dt, float inverseMass)
{
    const auto sphere = new GameObject();

    const auto sphereSize = Vector3(radius, radius, radius);
    const auto volume = new SphereVolume(radius);
    sphere->SetBoundingVolume(reinterpret_cast<CollisionVolume*>(volume));

    sphere->GetTransform()
        .SetScale(sphereSize)
        .SetPosition(position);

    sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
    sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

    sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
    sphere->GetPhysicsObject()->InitSphereInertia();

    sphere->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 5 * dt, 0));
    world->AddGameObject(sphere);

    return sphere;
}

void TutorialGame::InitOtherBall(const Vector3& position)
{
    constexpr float sphereRadius = 5.0f;
    const auto sphere = AddSphereToWorld(position, sphereRadius, 0);
    sphere->SetName("OtherBall");
    sphere->SetWorldID(105);
}

//Initialise game elements to the world Level1
void TutorialGame::InitGameElementsLevel1()
{
    InitTargetBall(Vector3(0, 5, 0));
    InitTargetEnding(Vector3(90, 10, -90));
    InitTargetControllerCube(Vector3(-30, 5, 30));
    InitOtherBall(Vector3(-50, 5, -50));
    InitConstraintCubeAndRotatingSphere(Vector3(-50, 50, -50), Vector3(-50, 50, -10));
    testStateObject = AddStateObjectToWorld(Vector3(20, 5, 10));
}

//Initialise sample characters and bonus frisbee
void TutorialGame::InitGameExamples() {
    AddPlayerToWorld(Vector3(0, 5, 0));
    AddEnemyToWorld(Vector3(5, 5, 0));
    AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
    float meshSize = 3.0f;
    float inverseMass = 0.5f;

    GameObject* character = new GameObject();

    AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

    character->SetBoundingVolume((CollisionVolume*)volume);

    character->GetTransform()
        .SetScale(Vector3(meshSize, meshSize, meshSize))
        .SetPosition(position);

    if (rand() % 2) {
        character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
    }
    else {
        character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
    }
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(character);

    //lockedObject = character;

    return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
    float meshSize = 3.0f;
    float inverseMass = 0.5f;

    GameObject* character = new GameObject();

    AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
    character->SetBoundingVolume((CollisionVolume*)volume);

    character->GetTransform()
        .SetScale(Vector3(meshSize, meshSize, meshSize))
        .SetPosition(position);

    character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
    character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

    character->GetPhysicsObject()->SetInverseMass(inverseMass);
    character->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(character);

    return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
    GameObject* apple = new GameObject();

    SphereVolume* volume = new SphereVolume(0.25f);
    apple->SetBoundingVolume((CollisionVolume*)volume);
    apple->GetTransform()
        .SetScale(Vector3(0.25, 0.25, 0.25))
        .SetPosition(position);

    apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
    apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

    apple->GetPhysicsObject()->SetInverseMass(1.0f);
    apple->GetPhysicsObject()->InitSphereInertia();

    world->AddGameObject(apple);

    return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
    if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
        inSelectionMode = !inSelectionMode;
        if (inSelectionMode) {
            Window::GetWindow()->ShowOSPointer(true);
            Window::GetWindow()->LockMouseToWindow(false);
        }
        else {
            Window::GetWindow()->ShowOSPointer(false);
            Window::GetWindow()->LockMouseToWindow(true);
        }
    }
    if (inSelectionMode) {
        renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

        if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
            if (selectionObject) {	//set colour to deselected;
                selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
                selectionObject = nullptr;
                lockedObject = nullptr;
            }

            Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

            RayCollision closestCollision;
            if (world->Raycast(ray, closestCollision, true)) {
                selectionObject = (GameObject*)closestCollision.node;
                selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
                return true;
            }
            else {
                return false;
            }
        }
    }
    else {
        renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
    }

    if (lockedObject) {
        renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
    }

    else if (selectionObject) {
        renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
    }

    if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
        if (selectionObject) {
            if (lockedObject == selectionObject) {
                lockedObject = nullptr;
            }
            else {
                lockedObject = selectionObject;
            }
        }

    }

    return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
    // Draw debug text at 10, 20
    //renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
    forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

    //We don't select anything
    if (!selectionObject)
        return;

    //Push the selected object
    if (Window::GetMouse()->ButtonPressed(MouseButtons::RIGHT))
    {
        Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
        RayCollision closestCollision;
        if (world->Raycast(ray, closestCollision, true))
        {
            if (closestCollision.node == selectionObject)
            {
                //selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);
                selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
            }
        }
    }
}
