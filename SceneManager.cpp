///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	//initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}
	//destroy the created OpenGL textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/**************************************************************
*  LoadSceneTextures()                                     
*  This method is used for preparing the 3D scene by loading 
*  the shapes, textures in memory to support the 3D scene 
*  rending
***************************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	//texture for ground plane 
	bReturn = CreateGLTexture(
		"textures/sand.jpg",
		"ground");
	//texture for backgound plane
	bReturn = CreateGLTexture(
		"textures/snowbackground.png",
		"snow");
	//texture for snowman
	bReturn = CreateGLTexture(
		"textures/snowman2.jpg",
		"snowman");
	//texture for carrot nose
	bReturn = CreateGLTexture(
		"textures/carrotnose.jpg",
		"nose");
	//texture for tophat
	bReturn = CreateGLTexture(
		"textures/tophat.jpg",
		"tophat");
	//texture for giftbox
	bReturn = CreateGLTexture(
		"textures/wrappingpaper.jpg",
		"giftbox");
	//texture for christmas tree
	bReturn = CreateGLTexture(
		"textures/tree.jpg",
		"tree");
	//texture for moon
	bReturn = CreateGLTexture(
		"textures/moon.jpg",
		"moon");
	//texture for turret
	bReturn = CreateGLTexture(
		"textures/turret.jpg",
		"turret");
	//texture for christmas lights
	bReturn = CreateGLTexture(
		"textures/purplelights.jpg",
		"purplelight");
	//texture for ornaments
	bReturn = CreateGLTexture(
		"textures/ornament.jpg",
		"ornaments");


	//after the texture image data is loaded into memory, the
	//loaded textures ned to be bound to texture slots - there
	//are a total of 16 available slots for scene textures
	BindGLTextures();
}

/**************************************************************
*  DefineObjectMaterials()
*
*  This method is used for configuring the various material
*  settings for all of the objects within the 3D scene.
* **************************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL bronzeMaterial;
	bronzeMaterial.diffuseColor = glm::vec3(0.714f, 0.4284f, 0.1814f);
	bronzeMaterial.specularColor = glm::vec3(0.393548f, 0.271906f, 0.166721f);
	bronzeMaterial.shininess = 20.0;
	bronzeMaterial.tag = "sand";

	m_objectMaterials.push_back(bronzeMaterial);
	
	OBJECT_MATERIAL silverMaterial;
	silverMaterial.diffuseColor = glm::vec3();
	silverMaterial.specularColor = glm::vec3();
	silverMaterial.shininess = 52.0;
	silverMaterial.tag = "silver";

	m_objectMaterials.push_back(silverMaterial);

	OBJECT_MATERIAL pearlMaterial;
	pearlMaterial.diffuseColor = glm::vec3(1.0f, 0.829f, 0.829f);
	pearlMaterial.specularColor = glm::vec3(0.296648f, 0.296648f, 0.296648f);
	pearlMaterial.shininess = 25.0;
	pearlMaterial.tag = "pearl";

	m_objectMaterials.push_back(pearlMaterial);

	OBJECT_MATERIAL copperMaterial;
	copperMaterial.diffuseColor = glm::vec3(0.7038f, 0.27048f, 0.0828f);
	copperMaterial.specularColor = glm::vec3(0.256777f, 0.137622f, 0.086014f);
	copperMaterial.shininess = 10.0;
	copperMaterial.tag = "carrot";

	m_objectMaterials.push_back(copperMaterial);

	OBJECT_MATERIAL blackMaterial;
	blackMaterial.diffuseColor = glm::vec3(0.01f, 0.01f, 0.01f);
	blackMaterial.specularColor = glm::vec3(0.50f, 0.50f, 0.50f);
	blackMaterial.shininess = 25.0;
	blackMaterial.tag = "hat";

	m_objectMaterials.push_back(blackMaterial);


	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	woodMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	woodMaterial.shininess = 0.1;
	woodMaterial.tag = "tree";

	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL redPlastic;
	redPlastic.diffuseColor = glm::vec3(0.5f, 0.0f, 0.0f);
	redPlastic.specularColor = glm::vec3(0.7f, 0.6f, 0.6f);
	redPlastic.shininess = 0.25;
	redPlastic.tag = "gift";

	m_objectMaterials.push_back(redPlastic);

	OBJECT_MATERIAL turqMaterial;
	turqMaterial.diffuseColor = glm::vec3(0.396f, 0.74151f, 0.69102f);
	turqMaterial.specularColor = glm::vec3(0.297254f, 0.30829f, 0.306678f);
	turqMaterial.shininess = 25.0;
	turqMaterial.tag = "ornament";

	m_objectMaterials.push_back(turqMaterial);

	OBJECT_MATERIAL goldMaterial;
	goldMaterial.diffuseColor = glm::vec3(0.75164f, 0.60648f, 0.22648f);
	goldMaterial.specularColor = glm::vec3(0.628281f, 0.555802f, 0.366065f);
	goldMaterial.shininess = 50.0;
	goldMaterial.tag = "lights";

	m_objectMaterials.push_back(goldMaterial);
}

/***************************************************************
*  SetupSceneLights()
*
*  This method is called to add and configure the light
*  sources for the 3D scene.
****************************************************************/

void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	//directional light
	m_pShaderManager->setVec3Value("directionalLight.direction", -13.0f, 17.0f, -7.0f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 1.0f, 1.0f, 1.0f);
	m_pShaderManager->setVec4Value("directionalLight.ambientColor", 0.373f, 0.5431, 0.91f, 1.0f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.6f, 0.6f, 0.6f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);


	//point light 1
	m_pShaderManager->setVec3Value("pointLights[0].position", 7.0f,5.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	
	//point light 2
	m_pShaderManager->setVec3Value("pointLights[1].position", 6.0f, 4.5f, -8.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.06f, 0.06f, 0.06f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

	//point light 3
	m_pShaderManager->setVec3Value("pointLights[2].position", -1.0f, 4.5f, 0.75f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[2].ambientColor", 0.7134f, 0.348f, 0.87f); 
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.06f, 0.06f, 0.06f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
	

	

	

	


}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/




void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	//load the texture image files for the textures applied
	//to objects in the 3D scene
	LoadSceneTextures();

	//define the materials for objects in the scene
	DefineObjectMaterials();
	//add and define the light sources for the scene
	SetupSceneLights();

	//load mesh shapes for scene
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.84f, 0.8019f, 0.7056f, 1.0f);//changed to sand color
	SetShaderTexture("ground");
	SetShaderMaterial("sand");
	//SetTextureUVScale(1.0, 1.0); // draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	//background mesh plane
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	
	//XYZ rotation for mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(0.0f, 9.0f, -10.0f);

	//set transformation into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//set color values
	SetShaderColor(0.1187f, 0.0986f, 0.34f, 1.0f);//Dark blue
	SetShaderTexture("snow");
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	
	//render snowman
	//first Sphere
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(2.5f, 2.5f, 2.5f);
	//XYZ rotation for mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	//XYZ position for mesh
	positionXYZ = glm::vec3(6.0f, 2.0f, 5.0f);
	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	SetShaderTexture("snowman");
	SetShaderMaterial("pearl");
	m_basicMeshes->DrawSphereMesh();


	//Second sphere
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(2.0f, 2.0f, 2.0f);
	//XYZ rotation for mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(6.0f, 5.0f, 5.0f);

	//set transformation into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("snowman");
	SetShaderMaterial("pearl");
	m_basicMeshes->DrawSphereMesh();

	//third sphere
	
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);
	
	//XYZ rotation for mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	
	//XYZ position for mesh
	positionXYZ = glm::vec3(6.0f, 7.5f, 5.0f);
	
	//Set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	SetShaderTexture("snowman");
	SetShaderMaterial("pearl");
	m_basicMeshes->DrawSphereMesh();

	//carrot nose
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 1.7f, 0.5f);
	
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 180.0f;
	ZrotationDegrees = 90.0f;
	
	//XYZ position for mesh
	positionXYZ = glm::vec3(5.0f, 7.5f, 6.0f);
	
	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	//set color values
	SetShaderColor(0.91f, 0.4345f, 0.0455f, 1.0f);//orange
	SetShaderTexture("nose");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("carrot");
	m_basicMeshes->DrawConeMesh();

	//render tophat
	//cylinder for the top hat
	
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(1.0f, 2.5f, 1.0f);
	
	//XYZ rotation for mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	
	//XYZ position for mesh
	positionXYZ = glm::vec3(6.0f, 11.0f, 5.0f);
	
	//set transformation into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	SetShaderTexture("tophat");
	SetShaderMaterial("hat");
	m_basicMeshes->DrawCylinderMesh();

	//cylinder for the brim of the tophat
	
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(1.5f, 0.25f, 1.5f);
	
	//XYZ rotations for mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -10.0f;
	
	//XYZ position for mesh
	positionXYZ = glm::vec3(6.0f, 9.0f, 5.0f);
	
	//set tranformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	SetShaderTexture("tophat");
	SetShaderMaterial("hat");
	m_basicMeshes->DrawCylinderMesh();

	//Christmas tree
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(4.5f, 16.0f, 4.5f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0f, 0.1f, -2.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("tree");
	SetShaderMaterial("tree");
	m_basicMeshes->DrawConeMesh();

	//Christmas present box
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(2.5f, 1.5f, 1.5f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = -40.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(7.0f, 1.0f, 8.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("giftbox");
	SetShaderMaterial("gift");
	m_basicMeshes->DrawBoxMesh();

	//moon
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(2.0f, 2.0f, 2.0f);

	//XYZ rotation for mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-13.0f, 17.0f, -7.0f);

	//set transformation into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//set shader texture
	SetShaderTexture("moon");
	SetShaderMaterial("silver");
	m_basicMeshes->DrawSphereMesh();

	//turret
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.8f, 0.8f, 3.0f);

	//XYZ rotation for mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(0.0f, 0.75f, 7.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	
	SetShaderTexture("turret");
	SetShaderMaterial("sand");
	m_basicMeshes->DrawTorusMesh();

	//christmas lights
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);
	
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0f, 1.5f, 2.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-1.0f, 1.5f, 1.6f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(0.75f, 1.5f, 0.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-5.0f, 1.5f, 1.6f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0f, 4.5f, 1.3f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-1.0f, 4.5f, 0.75f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-0.25f, 4.5f, -0.25f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-5.0f, 4.5f, 0.75f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0f, 7.5f, 0.5f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-1.0f, 7.5f, -0.5f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-5.0f, 7.5f, -0.4f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0f, 11.5f, -0.6f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-2.0f, 11.5f, -1.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.15f, 0.15f, 0.15f);

	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-4.0f, 11.5f, -1.0f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("purplelight");
	SetShaderMaterial("lights");
	m_basicMeshes->DrawSphereMesh();

	//ornaments
	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-2.0, 3.0f, 1.75f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	
	SetShaderTexture("ornaments");
	SetShaderMaterial("ornament");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-4.0f, 4.0f, 1.55f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("ornaments");
	SetShaderMaterial("ornament");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-1.25f, 6.0f, 0.5f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("ornaments");
	SetShaderMaterial("ornament");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-3.0, 9.0f, 0.25f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("ornaments");
	SetShaderMaterial("ornament");
	m_basicMeshes->DrawSphereMesh();

	//XYZ scale for mesh
	scaleXYZ = glm::vec3(0.3f, 0.3f, 0.3f);
	//XYZ rotation for mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	//XYZ position for mesh
	positionXYZ = glm::vec3(-2.75f, 12.75f, -0.8f);

	//set transformations into memory
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("ornaments");
	SetShaderMaterial("ornament");
	m_basicMeshes->DrawSphereMesh();
}
