// Copyright (C) 2002-2007 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_Q3_LEVEL_MESH_H_INCLUDED__
#define __C_Q3_LEVEL_MESH_H_INCLUDED__

#include "IQ3LevelMesh.h"
#include "IReadFile.h"
#include "IFileSystem.h"
#include "SMesh.h"
#include "IVideoDriver.h"
#include "irrString.h"
#include "ISceneManager.h"
#include "os.h"

namespace irr
{
namespace scene
{
	struct SMeshBufferLightMap;

	class CQ3LevelMesh : public IQ3LevelMesh
	{
	public:

		//! constructor
		CQ3LevelMesh(io::IFileSystem* fs, video::IVideoDriver* driver,  scene::ISceneManager* smgr);

		//! destructor
		virtual ~CQ3LevelMesh();

		//! loads a level from a .bsp-File. Also tries to load all needed textures. Returns true if successful.
		bool loadFile(io::IReadFile* file);

		//! returns the amount of frames in milliseconds. If the amount is 1, it is a static (=non animated) mesh.
		virtual s32 getFrameCount();

		//! returns the animated mesh based on a detail level. 0 is the lowest, 255 the highest detail. Note, that some Meshes will ignore the detail level.
		virtual IMesh* getMesh(s32 frameInMs, s32 detailLevel=255, s32 startFrameLoop=-1, s32 endFrameLoop=-1);

		virtual void releaseMesh ( s32 index );

		//! Returns an axis aligned bounding box of the mesh.
		//! \return A bounding box of this mesh is returned.
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! Returns the type of the animated mesh.
		virtual E_ANIMATED_MESH_TYPE getMeshType() const;

		//! loads the shader definition
		virtual const quake3::SShader * getShader ( const c8 * filename, s32 fileNameIsValid );

		//! returns a already loaded Shader
		virtual const quake3::SShader * getShader ( u32 index  ) const;


		//! get's an interface to the entities
		virtual const quake3::tQ3EntityList & getEntityList ();

	private:

		//! constructs a mesh from the quake 3 level file.
		void constructMesh();

		//! loads the textures
		void loadTextures();

		void constructMesh2();

		void loadTextures2();

		struct STexShader
		{
			video::ITexture* Texture;
			s32 ShaderID;
		};

		core::array< STexShader > Tex;
		core::array<video::ITexture*> Lightmap;

		enum eLumps
		{
			kEntities = 0,     // Stores player/object positions, etc...
			kTextures,         // Stores texture information
			kPlanes,           // Stores the splitting planes
			kNodes,            // Stores the BSP nodes
			kLeafs,            // Stores the leafs of the nodes
			kLeafFaces,        // Stores the leaf's indices into the faces
			kLeafBrushes,      // Stores the leaf's indices into the brushes
			kModels,           // Stores the info of world models
			kBrushes,          // Stores the brushes info (for collision)
			kBrushSides,       // Stores the brush surfaces info
			kVertices,         // Stores the level vertices
			kMeshVerts,        // Stores the model vertices offsets
			kShaders,          // Stores the shader files (blending, anims..)
			kFaces,            // Stores the faces for the level
			kLightmaps,        // Stores the lightmaps for the level
			kLightVolumes,     // Stores extra world lighting information
			kVisData,          // Stores PVS and cluster info (visibility)
			kMaxLumps          // A constant to store the number of lumps
		};

		struct tBSPLump
		{
			s32 offset;
			s32 length;
		}; 

		struct tBSPHeader
		{
			s32 strID;     // This should always be 'IBSP'
			s32 version;       // This should be 0x2e for Quake 3 files
		}; 

		struct tBSPVertex
		{
			f32 vPosition[3];      // (x, y, z) position. 
			f32 vTextureCoord[2];  // (u, v) texture coordinate
			f32 vLightmapCoord[2]; // (u, v) lightmap coordinate
			f32 vNormal[3];        // (x, y, z) normal vector
			u8 color[4];           // RGBA color for the vertex 
		};

		struct tBSPFace
		{
			s32 textureID;        // The index into the texture array 
			s32 effect;           // The index for the effects (or -1 = n/a) 
			s32 type;             // 1=polygon, 2=patch, 3=mesh, 4=billboard 
			s32 vertexIndex;      // The index into this face's first vertex 
			s32 numOfVerts;       // The number of vertices for this face 
			s32 meshVertIndex;    // The index into the first meshvertex 
			s32 numMeshVerts;     // The number of mesh vertices 
			s32 lightmapID;       // The texture index for the lightmap 
			s32 lMapCorner[2];    // The face's lightmap corner in the image 
			s32 lMapSize[2];      // The size of the lightmap section 
			f32 lMapPos[3];     // The 3D origin of lightmap. 
			f32 lMapBitsets[2][3]; // The 3D space for s and t unit vectors. 
			f32 vNormal[3];     // The face normal. 
			s32 size[2];          // The bezier patch dimensions. 
		};

		struct tBSPTexture
		{
			c8 strName[64];   // The name of the texture w/o the extension 
			u32 flags;          // The surface flags (unknown) 
			u32 contents;       // The content flags (unknown)
		};

		struct tBSPLightmap
		{
			u8 imageBits[128][128][3];   // The RGB data in a 128x128 image
		};

		struct tBSPNode
		{
			s32 plane;      // The index into the planes array 
			s32 front;      // The child index for the front node 
			s32 back;       // The child index for the back node 
			s32 mins[3];    // The bounding box min position. 
			s32 maxs[3];    // The bounding box max position. 
		}; 

		struct tBSPLeaf
		{
			s32 cluster;           // The visibility cluster 
			s32 area;              // The area portal 
			s32 mins[3];           // The bounding box min position 
			s32 maxs[3];           // The bounding box max position 
			s32 leafface;          // The first index into the face array 
			s32 numOfLeafFaces;    // The number of faces for this leaf 
			s32 leafBrush;         // The first index for into the brushes 
			s32 numOfLeafBrushes;  // The number of brushes for this leaf 
		}; 

		struct tBSPPlane
		{
			f32 vNormal[3];     // Plane normal. 
			f32 d;              // The plane distance from origin 
		};

		struct tBSPVisData
		{
			s32 numOfClusters;   // The number of clusters
			s32 bytesPerCluster; // Bytes (8 bits) in the cluster's bitset
			c8 *pBitsets;      // Array of bytes holding the cluster vis.
		}; 

		struct tBSPBrush 
		{
			s32 brushSide;           // The starting brush side for the brush 
			s32 numOfBrushSides;     // Number of brush sides for the brush
			s32 textureID;           // The texture index for the brush
		}; 

		struct tBSPBrushSide 
		{
			s32 plane;              // The plane index
			s32 textureID;          // The texture index
		}; 

		struct tBSPModel 
		{
			f32 min[3];           // The min position for the bounding box
			f32 max[3];           // The max position for the bounding box. 
			s32 faceIndex;          // The first face index in the model 
			s32 numOfFaces;         // The number of faces in the model 
			s32 brushIndex;         // The first brush index in the model 
			s32 numOfBrushes;       // The number brushes for the model
		}; 

		struct tBSPShader
		{
			c8 strName[64];     // The name of the shader file 
			s32 brushIndex;       // The brush index for this shader 
			s32 unknown;          // This is 99% of the time 5
		}; 

		struct tBSPLights
		{
			u8 ambient[3];     // This is the ambient color in RGB
			u8 directional[3]; // This is the directional color in RGB
			u8 direction[2];   // The direction of the light: [phi,theta] 
		}; 

		void loadTextures	(tBSPLump* l, io::IReadFile* file);		// Load the textures
		void loadLightmaps  (tBSPLump* l, io::IReadFile* file);      // Load the lightmaps
		void loadVerts      (tBSPLump* l, io::IReadFile* file);		// Load the vertices
		void loadFaces      (tBSPLump* l, io::IReadFile* file);		// Load the faces
		void loadPlanes     (tBSPLump* l, io::IReadFile* file);		// Load the Planes of the BSP
		void loadNodes      (tBSPLump* l, io::IReadFile* file);		// load the Nodes of the BSP
		void loadLeafs      (tBSPLump* l, io::IReadFile* file);		// load the Leafs of the BSP
		void loadLeafFaces  (tBSPLump* l, io::IReadFile* file);		// load the Faces of the Leafs of the BSP
		void loadVisData    (tBSPLump* l, io::IReadFile* file);		// load the visibility data of the clusters
		void loadEntities   (tBSPLump* l, io::IReadFile* file);		// load the entities
		void loadModels     (tBSPLump* l, io::IReadFile* file);		// load the models
		void loadMeshVerts  (tBSPLump* l, io::IReadFile* file);		// load the mesh vertices
		void loadBrushes    (tBSPLump* l, io::IReadFile* file);		// load the brushes of the BSP
		void loadBrushSides (tBSPLump* l, io::IReadFile* file);		// load the brushsides of the BSP
		void loadLeafBrushes(tBSPLump* l, io::IReadFile* file);		// load the brushes of the leaf
		void loadShaders	(tBSPLump* l, io::IReadFile* file);		// load the shaders

		// second parameter i is the zero based index of the current face.
		void createCurvedSurface(SMeshBufferLightMap* meshBuffer, s32 i);

		//bi-quadratic bezier patches
		void createCurvedSurface2 (	SMeshBufferLightMap* meshBuffer,
									s32 faceIndex,
									s32 patchTesselation,
									s32 storevertexcolor
								);

		void createCurvedSurface3 (	SMeshBufferLightMap* meshBuffer,
									s32 faceIndex,
									s32 patchTesselation,
									s32 storevertexcolor
								);

		f32 Blend( const f64 s[3], const f64 t[3], const tBSPVertex *v[9], int offset);

		struct S3DVertex2TCoords_64
		{
			core::vector3d<f64> Pos;
			core::vector3d<f64> Normal;
			video::SColorf Color;
			core::vector2d<f64> TCoords;
			core::vector2d<f64> TCoords2;

			void copyto ( video::S3DVertex2TCoords &dest ) const;

			S3DVertex2TCoords_64() {}
			S3DVertex2TCoords_64(const core::vector3d<f64>& pos, const core::vector3d<f64>& normal, const video::SColorf& color,
				const core::vector2d<f64>& tcoords, const core::vector2d<f64>& tcoords2)
				: Pos(pos), Normal(normal), Color(color), TCoords(tcoords), TCoords2(tcoords2) {}

			S3DVertex2TCoords_64 getInterpolated_quadratic(const S3DVertex2TCoords_64& v2, const S3DVertex2TCoords_64& v3, const f64 d) const
			{
				return S3DVertex2TCoords_64 (
						Pos.getInterpolated_quadratic ( v2.Pos, v3.Pos, d  ),
						Normal.getInterpolated_quadratic ( v2.Normal, v3.Normal, d ),
						Color.getInterpolated_quadratic ( v2.Color, v3.Color, (f32) d ),
						TCoords.getInterpolated_quadratic ( v2.TCoords, v3.TCoords, d ),
						TCoords2.getInterpolated_quadratic ( v2.TCoords2, v3.TCoords2, d )
					);
			}
		};

		void copy ( video::S3DVertex2TCoords * dest, const tBSPVertex * source, s32 vertexcolor ) const;
		void copy ( S3DVertex2TCoords_64 * dest, const tBSPVertex * source, s32 vertexcolor ) const;


		struct SBezier
		{
			SMeshBufferLightMap *Patch;
			S3DVertex2TCoords_64 control[9];

			void tesselate(s32 level);

		private:
			s32	Level;

			core::array<S3DVertex2TCoords_64> column[3];
			
		};
		SBezier Bezier;

		s32 PatchTesselation;

		tBSPLump Lumps[kMaxLumps];

		tBSPTexture* Textures;
		s32 NumTextures;

		tBSPLightmap* LightMaps;
		s32 NumLightMaps;

		tBSPVertex* Vertices;
		s32 NumVertices;

		tBSPFace* Faces;
		s32 NumFaces;

		tBSPPlane* Planes;
		s32 NumPlanes;

		tBSPNode* Nodes;
		s32 NumNodes;

		tBSPLeaf* Leafs;
		s32 NumLeafs;

		s32 *LeafFaces;
		s32 NumLeafFaces;

		s32 *MeshVerts;           // The vertex offsets for a mesh 
		s32 NumMeshVerts;

		tBSPBrush* Brushes;
		s32 NumBrushes;

		scene::SMesh* Mesh[quake3::E_Q3_MESH_SIZE];
		video::IVideoDriver* Driver;
		core::stringc LevelName;
		io::IFileSystem* FileSystem; // needs because there are no file extenstions stored in .bsp files.

		// Additional content
		scene::ISceneManager* SceneManager;
		enum eToken
		{
			Q3_TOKEN_UNRESOLVED	= 0,
			Q3_TOKEN_EOF		= 1,
			Q3_TOKEN_START_LIST,
			Q3_TOKEN_END_LIST,
			Q3_TOKEN_ENTITY,
			Q3_TOKEN_TOKEN,
			Q3_TOKEN_EOL,
			Q3_TOKEN_COMMENT,
			Q3_TOKEN_MATH_DIVIDE,
			Q3_TOKEN_MATH_ADD,
			Q3_TOKEN_MATH_MULTIPY
		};
		struct SQ3Parser
		{
			const c8 *source;
			u32 sourcesize;
			u32 index;
			core::stringc token;
			u32 tokenresult;
		};
		SQ3Parser Parser;


		typedef void ( CQ3LevelMesh::*tParserCallback ) ( quake3::SVarGroupList *& groupList );
		void parser_parse ( const void * data, u32 size, tParserCallback callback );
		void parser_nextToken ();

		void dumpVarGroup ( const quake3::SVarGroup * group, s32 stack ) const;

		void scriptcallback_entity ( quake3::SVarGroupList *& grouplist );
		quake3::tQ3EntityList Entity;

		void scriptcallback_shader ( quake3::SVarGroupList *& grouplist );
		core::array < quake3::SShader > Shader;
		quake3::tStringList ShaderFile;
		void InitShader ();
		void ReleaseShader ();
		void ReleaseEntity ();


		s32 setShaderMaterial ( video::SMaterial & material, const tBSPFace * face ) const;

		struct SToBuffer
		{
			s32 takeVertexColor;
			u32 index;
		};

		void cleanMeshes ();
		void calcBoundingBoxes ();

	};

} // end namespace scene
} // end namespace irr

#endif

