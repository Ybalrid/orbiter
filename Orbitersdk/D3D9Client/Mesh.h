// ==============================================================
// Mesh.h
// Part of the ORBITER VISUALISATION PROJECT (OVP)
// Dual licensed under GPL v3 and LGPL v3
// Copyright (C) 2006 Martin Schweiger
//				 2012-2014 Jarmo Nikkanen
// ==============================================================

// ==============================================================
// class D3D9Mesh (interface)
//
// This class represents a mesh in terms of DX7 interface elements
// (vertex buffers, index lists, materials, textures) which allow
// it to be rendered to the D3D9 device.
// ==============================================================

#ifndef __MESH_H
#define __MESH_H

#include "D3D9Client.h"
#include "D3D9Effect.h"
#include "AABBUtil.h"
#include <d3d9.h> 
#include <d3dx9.h>

const DWORD SPEC_DEFAULT = (DWORD)(-1); // "default" material/texture flag
const DWORD SPEC_INHERIT = (DWORD)(-2); // "inherit" material/texture flag


#define RENDER_VESSEL		0
#define RENDER_BASE			1
#define RENDER_ASTEROID		2
#define RENDER_BASETILES	3
#define RENDER_VC			4
#define RENDER_BASEBS		5

 
/**
 * \brief Mesh object with D3D9-specific vertex buffer
 *
 * Meshes consist of one or more vertex groups, and a set of materials and
 * textures.
 */

class D3D9Mesh : private D3D9Effect 
{

public:

	D9BBox BBox;

	D3DXMATRIX InstMatrix[8];
	
	struct GROUPREC {			// mesh group definition
		DWORD VertOff;			// Main mesh Vertex Offset
		DWORD FaceOff;			// Main mesh Index Offset
		DWORD BaseIdx;			// Main mesh Base Index
		//------------------------------------------------
		DWORD GeoVOff;			// Geometry Vertex Offset
		DWORD GeoFOff;			// Geometry Face Offset
		DWORD GeoBIdx;			// Geometry Base Index
		//------------------------------------------------
		DWORD nFace;			// Face count
		DWORD nVert;			// Vertex count
		//------------------------------------------------
		DWORD MtrlIdx;			// material index 0=None
		DWORD TexIdx;			// texture indices
		DWORD UsrFlag;			// user-defined flag
		WORD  IntFlag;			// internal flags
		WORD  InstanceRec;		// Instance record ID
		WORD  GeometryRec;		// Geometry record ID
		bool  bInstanced;		// Instancing helper flag.
		bool  bGrouped;			// Instancing helper flag.
		bool  bTransform;
		bool  bUpdate;
		D3DXMATRIX  Transform;	// Group specific transformation matrix
		D9BBox BBox;
		DWORD TexIdxEx[MAXTEX];
		float TexMixEx[MAXTEX];
	};

	struct INSTREC {
		DWORD  UsrFlag;
		DWORD  MtrlIdx;
		DWORD  TexIdx;
		DWORD  TexIdxEx;
		float  TexMixEx;
		DWORD  nGrp;
		DWORD  nVert;
		DWORD  nFace;
		DWORD  VertOff;
		DWORD  FaceOff;
		WORD   GrpIdx[8];
		bool   bBroken;
	};

	struct GEOMREC {
		DWORD  nVert;
		DWORD  nFace;
		DWORD  nGrp;
		DWORD  VertOff;
		DWORD  FaceOff;
		DWORD  Flags;
		WORD   GrpIdx[8];
		bool   bBroken;
		bool   bNoShadow;
	};

	
	D3D9Mesh(const D3D9Mesh &mesh);

	/**
	 * \brief Create a mesh consisting of a single mesh group
	 * \param client graphics client
	 * \param grp vertex group definition
	 * \param deepcopy if true, group contents are copied; otherwise, group
	 *   definition pointer is used directly
	 */
	D3D9Mesh(oapi::D3D9Client *client, DWORD nGrp, const MESHGROUPEX **hGroup, const SURFHANDLE *hSurf);
	D3D9Mesh(oapi::D3D9Client *client, const MESHGROUPEX *pGroup, const MATERIAL *pMat, D3D9ClientSurface *pTex);
	D3D9Mesh(oapi::D3D9Client *client, MESHHANDLE hMesh, bool asTemplate=false);
	~D3D9Mesh();

	void UnLockVertexBuffer();
	void UnLockIndexBuffer();
	NMVERTEX * LockVertexBuffer(DWORD grp);
	DWORD * LockIndexBuffer(DWORD grp);

	void SetName(const char *name);
	const char *GetName() const { return name; }

	/**
	 * \brief Returns number of vertex groups
	 * \return Number of groups
	 */
	inline DWORD GroupCount() const { return nGrp; }

	/**
	 * \brief Check if a mesh is casting shadows
	 * \return Returns true if the mesh is casting shadows.
	 */
	bool HasShadow();

	/**
	 * \brief Returns a pointer to a mesh group.
	 * \param idx group index (>= 0)
	 * \return Pointer to group structure.
	 */
	GROUPREC *GetGroup(DWORD idx);

	/**
	 * \brief Returns number of material specifications.
	 * \return Number of materials.
	 */
	inline DWORD MaterialCount() const { return nMtrl; }
	inline DWORD TextureCount() const { return nTex; }
	SURFHANDLE GetTexture(DWORD idx) const { return Tex[idx]; } 
	bool HasTexture(SURFHANDLE hSurf);
	bool IsReflective();

	/**
	 * \brief returns a pointer to a material definition.
	 * \param idx material index (>= 0)
	 * \return Pointer to material object.
	 */
	inline D3D9MatExt *GetMaterial(DWORD idx) { return Mtrl+idx; }
	

	// These two are only used by DebugControls.cpp
	DWORD GetMeshGroupMaterialIdx(DWORD grp);
	DWORD GetMeshGroupTextureIdx(DWORD grp);

	/**
	 * \brief Replace a mesh texture.
	 * \param texidx texture index (>= 0)
	 * \param tex texture handle
	 * \return \e true on success, \e false otherwise.
	 */
	bool SetTexture(DWORD texidx, LPD3D9CLIENTSURFACE tex);

	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer(DWORD grp);

	void SetTexMixture (DWORD ntex, float mix);

	
	void RenderGroup(LPDIRECT3DDEVICE9 dev, const GROUPREC *grp);
	void RenderMeshGroup(LPDIRECT3DDEVICE9 dev, DWORD Tech, DWORD idx, const LPD3DXMATRIX pW, LPD3D9CLIENTSURFACE pTex);
	void RenderBaseTile(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW);
	void RenderBoundingBox(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW);
	
	void RenderInstanced(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW, int iTech=RENDER_VESSEL, LPDIRECT3DCUBETEXTURE9 *pEnv=NULL, int nEnv=0);
	void Render(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW, int iTech=RENDER_VESSEL, LPDIRECT3DCUBETEXTURE9 *pEnv=NULL, int nEnv=0);
	void RenderBase(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW);
	void RenderShadows(LPDIRECT3DDEVICE9 dev, float alpha, const LPD3DXMATRIX pW);
	void RenderShadowsEx(LPDIRECT3DDEVICE9 dev, float alpha, const LPD3DXMATRIX pP, const LPD3DXMATRIX pW, const D3DXVECTOR4 *light, const D3DXVECTOR4 *param);
	void RenderAsteroid(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW);
	void RenderRings(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW, LPDIRECT3DTEXTURE9 pTex);
	void RenderRings2(LPDIRECT3DDEVICE9 dev, const LPD3DXMATRIX pW, LPDIRECT3DTEXTURE9 pTex, float irad, float orad);


	void ResetTransformations();
	void TransformGroup(DWORD n, const D3DXMATRIX *m);
	void Transform(const D3DXMATRIX *m);
	int  GetGroup (DWORD grp, GROUPREQUESTSPEC *grs);
	void DynamicGroup(DWORD idx);
	int  EditGroup (DWORD grp, GROUPEDITSPEC *ges);
	void UpdateGroupEx(DWORD idx, const MESHGROUPEX *mg);

	void SetSunLight(D3D9Light *pLight);
	
	D3D9Pick Pick(const LPD3DXMATRIX pW, const D3DXVECTOR3 *vDir);

	D3DXVECTOR3 GetBoundingSpherePos();
	float GetBoundingSphereRadius();

	D9BBox * GetAABB();
	
	void UpdateBoundingBox();
	void BoundingBox(const NMVERTEX *vtx, DWORD n, D9BBox *box);
	
	void SetAmbientColor(D3DCOLOR c);
	void SetupFog(const LPD3DXMATRIX pW);

	void DumpTextures();
	void DumpGroups();

	DWORD GetVertexCount() { return MaxVert; }
	D3DXVECTOR3 GetGroupSize(DWORD idx);
	LPD3DXMATRIX GetTransform() { if (bGlobalTF) return &mTransform; else return NULL; }

	/**
	 * \brief Enable/disable material alpha value for transparency calculation.
	 * \param enable flag for enabling/disabling material alpha calculation.
	 * \note By default, material alpha values are ignored for mesh groups
	 *   with textures, and the texture alpha values are used instead.
	 *   By enabling material alpha calculation, the final alpha value is
	 *   calculated as the product of material and texture alpha value.
	 */
	inline void EnableMatAlpha (bool enable) { bModulateMatAlpha = enable; }
	
private:

	void UpdateTangentSpace(NMVERTEX *pVrt, DWORD *pIdx, DWORD BaseOffset, DWORD nVtx, DWORD nFace, bool bTextured);
	void ProcessInstances();
	void ProcessInherit();
	bool CopyGroupEx(GROUPREC *grp, const MESHGROUPEX *mg, DWORD gid);
	void ClearGroups ();
	bool CopyMaterial (int idx, const MATERIAL *mat);
	void CheckValidity();
	void CreateGeometryBuffers();
	void UpdateGeometryBuffer();
	void Null();

	LPDIRECT3DVERTEXBUFFER9 pVB;
	LPDIRECT3DVERTEXBUFFER9 pGB;
	LPDIRECT3DINDEXBUFFER9  pIB;
	LPDIRECT3DINDEXBUFFER9  pGI;
	
	DWORD	MaxVert;
	DWORD	MaxFace;
	DWORD   Constr;

	oapi::D3D9Client *gc;		// the graphics client instance
	GROUPREC **Grp;             // list of mesh groups
	INSTREC *Inst;				// list of instance groups 
	GEOMREC *Geom;				// Geometry record
	DWORD nGrp;                 // number of mesh groups
	DWORD nGeom;				// number of geometry groups
	DWORD nInst;				// number of instance groups
	DWORD nMtrl;                // number of mesh materials
	DWORD nTex;                 // number of mesh textures
	D3D9MatExt *Mtrl;           // list of mesh materials
	LPD3D9CLIENTSURFACE *Tex;	// list of mesh textures
	D3DXMATRIX mTransform;
	D3DXMATRIX mTransformInv;
	D3DXMATRIX *pGrpTF;
	D3D9Light *sunLight;
	D3DCOLOR cAmbient;
	
	

	bool bTemplate;             // mesh used as template only (not for rendering)
	bool bBSRecompute;			// Bounding sphere must be recomputed
	bool bBSRecomputeAll;
	bool bModulateMatAlpha;     // mix material and texture alpha channels
	bool bGlobalTF;				// Mesh has a valid mTransform matrix

	char name[128];
};

#endif // !__MESH_H