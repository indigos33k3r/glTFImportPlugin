/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __gLTFImportExecutor_H__
#define __gLTFImportExecutor_H__

#include <map>
#include "hlms_editor_plugin.h"
#include "gLTFImportConstants.h"
#include "gLTFMaterial.h"
#include "gLTFImportAccessorsParser.h"
#include "gLTFImportMeshesParser.h"
#include "gLTFImportMaterialsParser.h"
#include "gLTFImportTexturesParser.h"
#include "gLTFImportImagesParser.h"
#include "gLTFImportSamplersParser.h"
#include "gLTFImportBuffersParser.h"
#include "gLTFImportBufferViewsParser.h"
#include "gLTFImportNodesParser.h"
#include "gLTFImportAnimationsParser.h"
#include "gLTFImportSkinsParser.h"
#include "gLTFImportPbsMaterialsCreator.h"
#include "gLTFImportOgreMeshCreator.h"
#include "rapidjson/document.h"

/** Class responsible for executing the import */
class gLTFImportExecutor
{
	public:
		gLTFImportExecutor (void);
		virtual ~gLTFImportExecutor(void) {};
		
		// Perform the import (called by plugin)
		bool executeImport(Ogre::HlmsEditorPluginData* data);

	protected:
		enum TextureTransformation
		{
			TTF_R000,				// Leave Red color intact and set Green, Blue and Alpha to zero
			TTF_0001,				// Set Alpha to 1.0
			TTF_R_2_GB,				// Green and Blue get the value of Red
			TTF_G_2_RB,				// Red and Blue get the value of Green
			TTF_B_2_RG,				// Red and Green get the value of Blue
			TTF_R_2_GB_INV,			// Red, Green and Blue get the value of 1 - Red
			TTF_G_2_RB_INV,			// Red, Green and Blue get the value of 1 - Green
			TTF_B_2_RG_INV,			// Red, Green and Blue get the value of 1 - Blue
			TTF_R_2_GBA,			// Green, Blue and Alpha get the value of Red
			TTF_G_2_RBA,			// Red, Blue and Alpha get the value of Green
			TTF_B_2_RGA,			// Red, Green and Alpha get the value of Blue
			TTF_A_2_RGBA,			// Red, Green, Blue and Alpha get the value of Alpha
		};

		// Process the binary file / text file
		bool executeBinary (const std::string& fileName, 
			Ogre::HlmsEditorPluginData* data,
			int& startBinaryBuffer); // proces .glb (binary) file
		bool executeText (const std::string& fileName, Ogre::HlmsEditorPluginData* data); // proces .gltf (json text) file
		bool executeJson (const std::string& fileName, 
			const char* jsonChar, 
			Ogre::HlmsEditorPluginData* data);

		// Data propagation (flatten the structure, so creation of Ogre 3d materials and meshes is less complex)
		bool propagateData (Ogre::HlmsEditorPluginData* data, 
			int startBinaryBuffer); // Arrange the data structure so it is easier to use when creating a Pbs material
		bool propagateBufferViews (void); // BufferViews are enriched with data from Buffers
		bool propagateMaterials (Ogre::HlmsEditorPluginData* data, int startBinaryBuffer); // Materials are enriched with data from BufferViews, Textures and Images
		bool propagateAccessors(void); // Accessors are enriched with data from Buffers and BufferViews
		bool propagateMeshes (Ogre::HlmsEditorPluginData* data); // Primitives are enriched with data from Materials and Accessors
		bool propagateNodes(Ogre::HlmsEditorPluginData* data);
		bool propagateAnimations (Ogre::HlmsEditorPluginData* data);
		bool propagateSkins(Ogre::HlmsEditorPluginData* data);
		gLTFNode* getTopLevelParentNode (gLTFNode* childNode); // Searches the highest parent in the tree
		void propagateNodeTransformsToChildren (gLTFNode* node); // Transforms are inherited from the parent
		
		// Utils
		const gLTFImage& getImageByTextureIndex (int index);
		int getImageIndexByTextureIndex (int index);
		const std::string& getImageUriByTextureIndex (int index);
		int getSamplerByTextureIndex (int index);
		const std::string& getMaterialNameByIndex (int index);
		void addAnimationsToNode (gLTFNode* node, int nodeIndex); // Add all animations in which a node participates
		const std::string& prepareUri(const std::string&  textureName,
			Ogre::HlmsEditorPluginData* data,
			const std::string&  materialName,
			int textureIndex,
			int startBinaryBuffer,
			bool& filewritten);
		bool setProjectFileNamePropertyValue (Ogre::HlmsEditorPluginData* data, const std::string& fileName);
		gLTFNode* findNodeByIndex (int nodeIndex);
		void writeTextureEntryToConfig (std::ofstream& texFile, const std::string& uriImage, int& texCount);
		const std::string& getImageIndexAsString (int textureIndex);
		gLTFAnimationSampler getAnimationSamplerByAnimationAndSamplerIndex (gLTFAnimation* animation, int samplerIndex);
		
		// Copy / extract and texture conversion
		const std::string& copyImageFile (const std::string& textureName,
			Ogre::HlmsEditorPluginData* data, 
			const std::string& materialName,
			const std::string& uriImage,
			bool& fileWritten);
		const std::string& extractAndCreateImageFromUriByTextureIndex(const std::string& textureName,
			Ogre::HlmsEditorPluginData* data,
			const std::string& materialName,
			int index,
			bool& filewritten);
		const std::string& extractAndCreateImageFromFileByTextureIndex (const std::string& textureName,
			Ogre::HlmsEditorPluginData* data, 
			const std::string& materialName,
			int index,
			int startBinaryBuffer,
			bool& filewritten);
		bool convertTexture (const std::string& fileName, 
			TextureTransformation transformation); // Convert an image file
		const std::string& writeImageFile (const std::string& textureName,
			const std::string& mimeType,
			Ogre::HlmsEditorPluginData* data,
			const std::string& materialName,
			char* imageBlock,
			size_t byteLength,
			bool& filewritten);
		bool isDuplicateUri (const std::string& uri); // Check whether the image file was already written/copied

	private:
		bool mHasAnimations; // If true, the gLTF file has animations
		std::string mHelperString;
		std::string mHelperMaterialNameString;
		Ogre::Matrix4 mHelperMatrix4;
		gLTFImage mHelperImage;
		std::string mHelperUri;
		std::string mHelperOutputFile;
		gLTFAnimation mHelperAnimation;
		gLTFAnimationSampler mHelperAnimationSampler;
		std::map<int, gLTFAccessor> mAccessorsMap;
		std::map<int, gLTFMesh> mMeshesMap;
		std::map<int, gLTFMaterial> mMaterialsMap;
		std::map<int, gLTFTexture> mTexturesMap;
		std::map<int, gLTFImage> mImagesMap;
		std::map<int, gLTFSampler> mSamplersMap;
		std::map<int, gLTFBufferView> mBufferViewsMap;
		std::map<int, gLTFBuffer> mBuffersMap;
		std::map<int, gLTFNode> mNodesMap;
		std::map<int, gLTFAnimation> mAnimationsMap;
		std::map<int, gLTFSkin> mSkinsMap;

		gLTFImportAccessorsParser mAccessorsParser;
		gLTFImportMeshesParser mMeshesParser;
		gLTFImportMaterialsParser mMaterialsParser;
		gLTFImportTexturesParser mTexturesParser;
		gLTFImportSamplersParser mSamplersParser;
		gLTFImportBuffersParser mBuffersParser;
		gLTFImportBufferViewsParser mBufferViewsParser;
		gLTFImportImagesParser mImagesParser;
		gLTFImportNodesParser mNodesParser;
		gLTFImportAnimationsParser mAnimationsParser;
		gLTFImportSkinsParser mSkinsParser;
		
		// Creator classes
		gLTFImportPbsMaterialsCreator mPbsMaterialsCreator;
		gLTFImportOgreMeshCreator mOgreMeshCreator;

		// Filenames project
		std::string mHlmsProjectFileName;
		std::string mMaterialsConfigFileName;
		std::string mTexturesConfigFileName;
		std::string mMeshesConfigFileName;
};

#endif
