#include"Player.h"
#include"OBJLoader.h"
#include"SphereCollider.h"
#include"Input.h"
#include"DebugText.h"
#include"CollisionManager.h"
#include"CollisionAttribute.h"
#include "Easing.h"
#include"FBXManager.h"
DebugCamera* Player::camera = nullptr;
LightCamera* Player::lightCamera = nullptr;

Player::Player()
{
	Create(FBXManager::GetModel("Hidari2"));
	naObject = new Object();
	naObject->Create(FBXManager::GetModel("Hidari1"));
	Initialize();
}

void Player::Initialize()
{
	name = typeid(*this).name();
	//scale = { 0.3f,1,0.5f };
	position = { 5,2,5 };
	rotation = { 90,0,0 };
	naObject->SetPosition(position);
	naObject->SetRotation(rotation);
	prePos = position;
	nowAttackState = NONE;
	lightCamera->SetTarget(position + Vector3{ 0,1,0 });
	camera->SetTarget(position + Vector3{ 0, 1, 0 });

}

void Player::Update()
{
	Move();
	//落下処理
	if (!onGround)
	{
		//下向き加速
		const float fallAcc = -0.02f;
		const float fallVYMin = -0.5f;
		//加速
		fallV.m128_f32[1] = max(fallV.m128_f32[1] + fallAcc, fallVYMin);
		//移動
		position.x += fallV.m128_f32[0];
		position.y += fallV.m128_f32[1];
		position.z += fallV.m128_f32[2];
	}
	//ジャンプ動作
	else if (Input::TriggerKey(DIK_SPACE))
	{
		onGround = false;
		const float jumpVYFist = 0.5f;//ジャンプ時上向き初速
		fallV = { 0,jumpVYFist,0,0 };
	}
	
	//コライダー更新	
	Object::Update();
	naObject->Update();
	if (prePos != position)
	{
		lightCamera->SetTarget(position + Vector3{0,1,0});
		camera->SetTarget(position + Vector3{0, 1, 0});
	}
}

void Player::OnCollision(const CollisionInfo & info)
{
	SphereCollider* sphereCollider = dynamic_cast<SphereCollider*>(collider);
	assert(sphereCollider);
	sphereCollider->SetRadius(0.50f);
	sphereCollider->SetOffset({ 0,0.5f,0 ,0 });
	//クエリーコールバッククラス
	class PlayerQueryCallBack
	{
	public:
		PlayerQueryCallBack(Sphere* sphere) :sphere(sphere) {};

		//衝突時のコールバック関数
		bool OnQueryHit(const CollisionInfo& info)
		{
			//ワールドの上方向
			const XMVECTOR up = { 0,1,0,0 };
			//排斥方向
			XMVECTOR rejectDir = XMVector3Normalize(info.reject);
			//上方向と排斥方向の角度差のコサイン値
			float cos = XMVector3Dot(rejectDir, up).m128_f32[0];

			//地面判定しきい値角度
			const float threshold = cosf(XMConvertToRadians(30.0f));
			//角度差によって天井または地面と判定される場合を除いて
			if (-threshold < cos && cos < threshold)
			{
				//球を排斥(押し出す)
				sphere->center += info.reject;
				move += info.reject;
			}
			return true;
		}
		//クエリーに使用する球
		Sphere* sphere = nullptr;
		//排斥による移動量(累積値)
		DirectX::XMVECTOR move = {};
	};

	//クエリーコールバックの関数オブジェクト
	PlayerQueryCallBack callback(sphereCollider);
	callback.OnQueryHit(info);
	//球と地形の交差を全検索
	//CollisionManager::GetInstance()->QuerySphere(*sphereCollider, &callback, COLLISION_ATTR_LANDSHAPE);

	//交差による排斥文を動かす
	position.x += callback.move.m128_f32[0];
	position.y += callback.move.m128_f32[1];
	position.z += callback.move.m128_f32[2];
}

void Player::Draw()
{
	DirectXLib::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Object::CustomDraw(true);
	naObject->CustomDraw(true);
}

void Player::Attack()
{
	
}

void Player::Move()
{
	prePos = position;
	if (Input::DownKey(DIK_D) || Input::DownKey(DIK_A))
	{
		if (Input::DownKey(DIK_D))
		{
			position.x += speed;
		}
		if (Input::DownKey(DIK_A))
		{
			position.x -= speed;
		}

		if (nowAttackState == NONE)
			naObject->SetPosition(position);
	}
}
