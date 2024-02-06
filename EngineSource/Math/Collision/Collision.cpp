#include "Collision.h"
#include <algorithm>
#include <Math/Collision/TriangleIntersect.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Log.h>
#include <cmath>
#include <Math/Math.h>
#include <Math/Physics/Physics.h>

namespace Collision
{
	std::vector<CollisionComponent*> CollisionBoxes;
}

bool Collision::CollisionAABB(Box a, Box b)
{
	return (a.minX <= b.maxX && a.maxX >= b.minX) &&
		(a.minY <= b.maxY && a.maxY >= b.minY) &&
		(a.minZ <= b.maxZ && a.maxZ >= b.minZ);
}


Collision::HitResponse testRayThruTriangle(glm::vec3 orig, glm::vec3 end, glm::vec3 A, glm::vec3 B, glm::vec3 C)
{
	glm::vec3 dir = end;
	glm::vec3 E1 = C - A;
	glm::vec3 E2 = B - A;
	glm::vec3 N = glm::cross(E1, E2);
	float det = -glm::dot(dir, N);
	float invdet = 1.0f / det;
	glm::vec3 AO = orig - A;
	glm::vec3 DAO = glm::cross(AO, dir);
	float u = dot(E2, DAO) * invdet;
	float v = -dot(E1, DAO) * invdet;
	float t = dot(AO, N) * invdet;
	if (( t >= 0.0f && u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f))
		return Collision::HitResponse(true, orig + end * t, normalize(N), t);
	else return Collision::HitResponse();
}

Collision::HitResponse Collision::TriangleLine(const Vector3& TriA, const Vector3& TriB, const Vector3& TriC, const Vector3& RayStart, const Vector3& RayEnd)
{
	return testRayThruTriangle(RayStart, RayEnd - RayStart, TriA, TriB, TriC);
}


bool SpheresOverlapping(glm::vec3 Sphere1Pos, float Sphere1Radius, glm::vec3 Sphere2Pos, float Sphere2Radius)
{
	return glm::distance(Sphere1Pos, Sphere2Pos) < Sphere1Radius + Sphere2Radius;
}

namespace Collision
{
	float max3(float a, float b, float c)
	{
		return std::max(std::max(a, b), c);
	}

	float min3(float a, float b, float c)
	{
		return std::min(std::min(a, b), c);
	}
}

/*Collision::HitResponse Collision::BoxOverlapCheck(Collision::Box a, Vector3 Offset)
{
	a = a + Offset;
	Collision::HitResponse r;
	for (int i = 0; i < Collision::CollisionBoxes.size(); i++)
	{
		if (SpheresOverlapping(a.GetCenter(), a.GetExtent().Length(),
			Collision::CollisionBoxes[i]->CollMesh->WorldPosition, Collision::CollisionBoxes[i]->CollMesh->SphereCollisionSize))
		{
			CollisionMesh* m = Collision::CollisionBoxes[i]->CollMesh;
			for (size_t i = 0; i < m->Indices.size(); i += 3)
			{
				Vector3 v0 = m->Vertices[m->Indices[i]].Position, v1 = m->Vertices[m->Indices[i + 1]].Position, v2 = m->Vertices[m->Indices[i + 2]].Position;
				bool NewCollision = TriangleAABBCollision(v0, v1, v2, a);
				if (NewCollision)
				{
					//Log::Print(v0.ToString() + " - " + a.GetCenter().ToString());

					r.Hit = true;
					Vector3 n = (Vector3::Cross(v1 - v0, v2 - v0));
					if (isnan(n.X))
					{
						r.Hit = false;
						//the triangle has a surface area of 0, we skip it (if thats the case, all elements of the normal vector are nan)
						continue;
					}
					r.Normal += n.Normalize();
				}
			}
		}
	}
	r.Normal = r.Normal.Normalize();
	return r;
}*/


bool Collision::IsPointIn3DBox(Box a, Vector3 p)
{
	return (a.minX <= p.X && a.maxX >= p.X) &&
		(a.minY <= p.Y && a.maxY >= p.Y) &&
		(a.minZ <= p.Z && a.maxZ >= p.Z);
}
//https://gamedev.stackexchange.com/questions/96459/fast-ray-sphere-collision-code
// Intersects ray r = p + td, |d| = 1, with sphere s and, if intersecting, 
// returns t value of intersection and intersection point q 
Collision::HitResponse IntersectRaySphere(Vector3 Start, Vector3 Dir, Vector3 sphere, float radius)
{
	return Collision::HitResponse(true, 0, Vector3(), 0, nullptr);
	Vector3 m = Start - sphere;
	float b = Vector3::Dot(m, Dir);
	float c = Vector3::Dot(m, m) - radius * radius;
	// Exit if r�s origin outside s (c > 0) and r pointing away from s (b > 0) 
	if (c > 0.0f && b > 0.0f) return Collision::HitResponse();
	float discr = b * b - c;

	// A negative discriminant corresponds to ray missing sphere 
	//if (discr < 0.0f) return Collision::HitResponse();

	// Ray now found to intersect sphere, compute smallest t value of intersection
	float t = -b - sqrt(discr);

	// If t is negative, ray started inside sphere so clamp t to zero 
	if (t < 0.0f) t = 0.0f;
	Vector3 q = Start + t * Dir;

	return Collision::HitResponse(true, q, Vector3(), t, nullptr);
}


Collision::HitResponse Collision::LineTrace(Vector3 RayStart, Vector3 RayEnd, std::set<WorldObject*> ObjectsToIgnore, std::set<CollisionComponent*> MeshesToIgnore)
{
	HitResponse Result;

	auto Hit = Physics::RayCast(RayStart, RayEnd);

	Result.Hit = Hit.Hit;
	Result.HitComponent = Hit.HitComponent;
	Result.ImpactPoint = Hit.ImpactPoint;
	if (Hit.HitComponent)
	{
		Result.HitObject = Hit.HitComponent->GetParent();
	}
	return Result;
}

//send help
Collision::HitResponse Collision::LineCheckForAABB(Collision::Box a, Vector3 RayStart, Vector3 RayEnd)
{
	Vector3 Normal;
	Vector3 RayDir = RayEnd - RayStart;
	float tmp;

	float txMin = (a.minX - RayStart.X) / RayDir.X;
	float txMax = (a.maxX - RayStart.X) / RayDir.X;
	if (txMax < txMin)
	{
		tmp = txMax;
		txMax = txMin;
		txMin = tmp;
	}
	float tyMin = (a.minY - RayStart.Y) / RayDir.Y;
	float tyMax = (a.maxY - RayStart.Y) / RayDir.Y;
	if (tyMax < tyMin)
	{
		tmp = tyMax;
		tyMax = tyMin;
		tyMin = tmp;
	}
	float tzMin = (a.minZ - RayStart.Z) / RayDir.Z;
	float tzMax = (a.maxZ - RayStart.Z) / RayDir.Z;
	if (tzMax < tzMin)
	{
		tmp = tzMax;
		tzMax = tzMin;
		tzMin = tmp;
	}
	float tMin;
	if (txMin > tyMin)
	{
		tMin = txMin;
		Normal = Vector3(-1, 0, 0);
	}
	else
	{
		tMin = tyMin;
		Normal = Vector3(0, -1, 0);
	}
	float tMax = (txMax < tyMax) ? txMax : tyMax;
	if (txMin > tyMax || tyMin > txMax) return HitResponse();
	if (tMin > tzMax || tzMin > tMax) return HitResponse();
	if (tzMin > tMin)
	{
		tMin = tzMin;
		Normal = Vector3(0, 0, -1);
	}
	if (tMax < tzMax) tMax = tzMax;
	Vector3 ImpactLocation = (RayDir * tMin);
	Vector3 RayDirNormalized = Vector3(RayDir.X > 0.0f ? 1.0f : -1.0f, RayDir.Y > 0.0f ? 1.0f : -1.0f, RayDir.Z > 0.0f ? 1.0f : -1.0f);
	Normal = Normal * RayDirNormalized;
	if (tMin < 0)
	{
		if (!IsPointIn3DBox(a, RayStart))
		{
			return HitResponse();
		}
	}
	return HitResponse(true, ImpactLocation + RayStart, Normal, tMin);
}


Collision::CollisionMesh::CollisionMesh(std::vector<Vertex> Verts, std::vector<unsigned int> Indices, Transform T)
{
	RawVertices = Verts;
	this->Indices = Indices;
	ModelMatrix = T.ToMatrix();
	Scale = T.Scale;
	WorldPosition = T.Location;
	WorldScale = std::max(std::max(T.Scale.X, T.Scale.Y), T.Scale.Z);

	ApplyMatrix();
}

Collision::CollisionMesh::~CollisionMesh()
{
	Vertices.clear();
	RawVertices.clear();
	Indices.clear();
}

glm::mat4 Collision::CollisionMesh::GetMatrix()
{
	return ModelMatrix;
}

void Collision::CollisionMesh::SetTransform(Transform T)
{
	glm::mat4 newM = T.ToMatrix();
	if (newM != ModelMatrix)
	{
		ModelMatrix = newM;
		WorldPosition = T.Location;
		WorldScale = std::max(std::max(T.Scale.X, T.Scale.Y), T.Scale.Z);
		Scale = T.Scale;

		ApplyMatrix();
	}
}

void Collision::CollisionMesh::ApplyMatrix()
{
	Vertices.clear();
	Vertices.resize(RawVertices.size());
	SpherePosition = 0;
	for (size_t i = 0; i < RawVertices.size(); i++)
	{
		Vertices[i] = RawVertices[i];
		Vertices[i].Position = ModelMatrix * glm::vec4(Vertices[i].Position, 1);
		Vertices[i].Normal = glm::normalize(glm::mat3(ModelMatrix) * Vertices[i].Normal);
		SpherePosition += Vertices[i].Position;
	}
	SpherePosition = (SpherePosition / (float)Vertices.size()) - WorldPosition;
	SphereCollisionSize = 0;
	glm::vec3 Furthest = glm::vec3(0);
	for (size_t i = 0; i < RawVertices.size(); i++)
	{
		float dist = glm::distance(RawVertices[i].Position * (glm::vec3)Scale, (glm::vec3)SpherePosition);
		if (dist > SphereCollisionSize)
		{
			Furthest = RawVertices[i].Position;
			SphereCollisionSize = dist;
		}
	}
}
//Wow, this function sucks
Collision::HitResponse Collision::CollisionMesh::CheckAgainstMesh(CollisionMesh* b)
{
	if (!CanOverlap) return Collision::HitResponse();
	if (!b->CanOverlap) return Collision::HitResponse();
	if (SpheresOverlapping(WorldPosition + SpherePosition, SphereCollisionSize,
		b->WorldPosition + b->SpherePosition, b->SphereCollisionSize))
	{
		HitResponse r;
		r.Hit = false;
		for (unsigned int i = 0; i < Indices.size(); i += 3)
		{
			for (int32_t j = 0; j < b->Indices.size(); j += 3)
			{
				
				auto& Tri2A = b->Vertices[b->Indices[j]].Position;
				auto& Tri2B = b->Vertices[b->Indices[j + 1]].Position;
				auto& Tri2C = b->Vertices[b->Indices[j + 2]].Position;
				
				float* a1[3], *b1[3], *c1[3], *a2[3], *b2[3], *c2[3];

				auto& Tri1A = Vertices[Indices[i]].Position;
				auto& Tri1B = Vertices[Indices[i + 1]].Position;
				auto& Tri1C = Vertices[Indices[i + 2]].Position;

				//Tri 1
				a1[0] = &Tri1A.x; a1[1] = &Tri1A.y; a1[2] = &Tri1A.z;
				b1[0] = &Tri1B.x; b1[1] = &Tri1B.y; b1[2] = &Tri1B.z;
				c1[0] = &Tri1C.x; c1[1] = &Tri1C.y; c1[2] = &Tri1C.z;
				//Tri 2
				a2[0] = &Tri2A.x; a2[1] = &Tri2A.y; a2[2] = &Tri2A.z;
				b2[0] = &Tri2B.x; b2[1] = &Tri2B.y; b2[2] = &Tri2B.z;
				c2[0] = &Tri2C.x; c2[1] = &Tri2C.y; c2[2] = &Tri2C.z;
				if (tri_tri_overlap_test_3d(a1, b1, c1, a2, b2, c2))
				{
					r.Hit = true;
					Vector3 n = Vector3::Cross(
						Vector3(*b2[0], *b2[1], *b2[2]) - Vector3(*a2[0], *a2[1], *a2[2]),
						Vector3(*c2[0], *c2[1], *c2[2]) - Vector3(*a2[0], *a2[1], *a2[2]));
					if (std::isnan(n.X))
					{
						r.Hit = false;
						//the triangle has a surface area of 0, we skip it (if thats the case, all elements of the normal vector are nan)
						continue;
					}

					r.Normal += n;
				}

			}
		}
		if (r.Hit)
		{
			r.Normal = r.Normal.Normalize();
			return r;
		}
	}

	return Collision::HitResponse();
}

Collision::HitResponse Collision::CollisionMesh::CheckAgainstLine(Vector3 RayStart, Vector3 RayEnd)
{
	Box BroadPhaseBox = Collision::Box(
		SpherePosition.X - SphereCollisionSize, SpherePosition.X + SphereCollisionSize,
		SpherePosition.Y - SphereCollisionSize, SpherePosition.Y + SphereCollisionSize,
		SpherePosition.Z - SphereCollisionSize, SpherePosition.Z + SphereCollisionSize).TransformBy(Transform(WorldPosition, Vector3(), Vector3(1)));
	if (LineCheckForAABB(BroadPhaseBox,
		RayStart, RayEnd).Hit)
	{
		Vector3 Dir = RayEnd - RayStart;
		HitResponse r;
		r.Hit = false;
		r.t = INFINITY;

		for (size_t i = 0; i < Indices.size(); i += 3)
		{
			Vector3 CurrentTriangle[3] = { Vertices[Indices[i]].Position, Vertices[Indices[i + 2]].Position, Vertices[Indices[i + 1]].Position };
			HitResponse newR
				= (testRayThruTriangle(RayStart, Dir, CurrentTriangle[0], CurrentTriangle[1], CurrentTriangle[2]));
			if (newR.t < r.t && newR.Hit && newR.t < 1)
			{
				r = newR;
			}
		}
		return r;
	}
	return HitResponse();
}

Collision::HitResponse Collision::CollisionMesh::CheckAgainstAABB(const Box& b)
{
	return HitResponse();
}
Collision::HitResponse Collision::CollisionMesh::OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore)
{
	/*
	for (CollisionComponent* c : CollisionBoxes)
	{
		if (MeshesToIgnore.find(c) == MeshesToIgnore.end())
		{
			if (c->Collider != this)
			{
				HitResponse newR = CheckAgainstMesh(c->Collider);
				if (newR.Hit)
				{
					newR.HitObject = c->GetParent();
					newR.HitComponent = c;
					return newR;
				}
			}
		}
	}
	*/
	return HitResponse();
}