#define PI 3.1415926535897932384626433832795

//XPLM
#include "XPLMCamera.h"
#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMInstance.h"
#include "XPLMMap.h"
#include "XPLMMenus.h"
#include "XPLMNavigation.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMScenery.h"
#include "XPLMUtilities.h"

//Widgets
#include "XPStandardWidgets.h"
#include "XPUIGraphics.h"
#include "XPWidgetDefs.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"

//Wrappers
#include "XPCBroadcaster.h"
#include "XPCDisplay.h"
#include "XPCListener.h"
#include "XPCProcessing.h"
#include "XPCWidget.h"
#include "XPCWidgetAttachments.h"

#if IBM
	#include <gl/gl.h>
	#include <gl/glu.h>
	#include <windows.h>
#endif


#include <iostream>
#include <fstream>
#include <string>

#include "tinyxml2.h"

XPLMObjectRef ObjetoNormal = NULL;
XPLMObjectRef ObjetoTemp = NULL;
XPLMObjectRef ObjetoDef = NULL;

std::string pathConfigFile = "./Resources/plugins/CIPqdt_Master/config.xml";
std::string pathObjNormal = "./Resources/plugins/CIPqdt_Master/objetos/normal_letra.obj";
std::string pathObjTemp = "./Resources/plugins/CIPqdt_Master/objetos/normal_adv.obj";
std::string pathObjDef = "./Resources/plugins/CIPqdt_Master/objetos/letra_vermelha_amarela.obj";
XPLMInstanceRef normalObjInstance, tempObjInstance, defObjInstance;

double lat = -22.87097409496356;
double lon = -43.37670666749845;
double azimute = 0;
double alt = 10000;
double X = 0;
double Y = 0;
double Z = 0;
XPLMProbeRef ProbeRef = NULL;
bool objNormalCarregado = false;
bool objImpTempCarregado = false;
bool objImpDefCarregado = false;

static XPLMKeyFlags	gFlags = 0;
static char			gVirtualKey = 0;
static char			gChar = 0;

static void menu_plugin( void* inMenuRef, void* inItemRef);

void LeArquivoConfig();

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	strcpy(outName, "CIPqdt-Master");
	strcpy(outSig, "COTER.CIPqdt-Master");
	strcpy(outDesc, "Plugin para o adestramento de mestre de salto do Centro de Instrução Pára-quedista.");
	LeArquivoConfig();
	if (ObjetoNormal == NULL)
	{
		ObjetoNormal = XPLMLoadObject(pathObjNormal.c_str());
		if (ObjetoNormal == NULL)
		{
			objNormalCarregado = false;
		}
		else
		{
			objNormalCarregado = true;
		}
	}
	if (ObjetoTemp == NULL)
	{
		ObjetoTemp = XPLMLoadObject(pathObjTemp.c_str());
		if (ObjetoTemp == NULL)
		{
			objImpTempCarregado = false;
		}
		else
		{
			objImpTempCarregado = true;
		}
	}
	if (ObjetoDef == NULL)
	{
		ObjetoDef = XPLMLoadObject(pathObjDef.c_str());
		if (ObjetoDef == NULL)
		{
			objImpDefCarregado = false;
		}
		else
		{
			objImpDefCarregado = true;
		}
	}
	int meu_slot = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Centro de Instrução Pára-quedista (Master)", NULL, 0);
	XPLMMenuID menuId = XPLMCreateMenu("CIPqdt-Master", XPLMFindPluginsMenu(), meu_slot, menu_plugin, NULL);
	XPLMAppendMenuItem(menuId, "Adicionar objeto normal", "01", 0);
	XPLMAppendMenuItem(menuId, "Adicionar objeto impedimento temporário", "02", 0);
	XPLMAppendMenuItem(menuId, "Adicionar objeto impedimento definitivo", "03", 0);
	XPLMAppendMenuItem(menuId, "Remover todos os objetos", "04", 0);
	XPLMAppendMenuItem(menuId, "Carregar arquivo de configuração", "05", 0);
	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
	XPLMUnloadObject(ObjetoNormal);
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, long inMsg, void * inParam)
{
}

static void menu_plugin(
	void* inMenuRef,
	void* inItemRef) 
{
	// cria probe
	ProbeRef = XPLMCreateProbe(xplm_ProbeY);
	XPLMWorldToLocal(lat, lon, alt, &X, &Y, &Z);

	XPLMProbeInfo_t info;
	info.structSize = sizeof(info);
	// Probe the terrain
	XPLMProbeResult result = XPLMProbeTerrainXYZ( ProbeRef, X, Y, Z, &info);
	XPLMDrawInfo_t coordenadas_OGL[1] = { 0 };
	coordenadas_OGL[0].structSize = sizeof(XPLMDrawInfo_t);
	coordenadas_OGL[0].x = info.locationX;
	coordenadas_OGL[0].y = info.locationY + 2; // 2 metros acima do solo
	coordenadas_OGL[0].z = info.locationZ;
	coordenadas_OGL[0].pitch = 0;
	coordenadas_OGL[0].heading = azimute;
	coordenadas_OGL[0].roll = 0;

	const char* null_ref[] = {NULL};
	if (!strcmp((const char*)inItemRef, "01") && objNormalCarregado)
	{
		normalObjInstance = XPLMCreateInstance(ObjetoNormal, null_ref);
		XPLMInstanceSetPosition(normalObjInstance, coordenadas_OGL, 0);
		XPLMDestroyInstance(tempObjInstance);
		XPLMDestroyInstance(defObjInstance);
	}
	else if (!strcmp((const char*)inItemRef, "02") && objImpTempCarregado)
	{
		tempObjInstance = XPLMCreateInstance(ObjetoTemp, null_ref);
		XPLMInstanceSetPosition(tempObjInstance, coordenadas_OGL, 0);
		XPLMDestroyInstance(normalObjInstance);
		XPLMDestroyInstance(defObjInstance);
	}
	else if (!strcmp((const char*)inItemRef, "03") && objImpDefCarregado)
	{
		defObjInstance = XPLMCreateInstance(ObjetoDef, null_ref);
		XPLMInstanceSetPosition(defObjInstance, coordenadas_OGL, 0);
		XPLMDestroyInstance(tempObjInstance);
		XPLMDestroyInstance(normalObjInstance);
	}
	else if (!strcmp((const char*)inItemRef, "04")) 
	{
		XPLMDestroyInstance(normalObjInstance);
		XPLMDestroyInstance(tempObjInstance);
		XPLMDestroyInstance(defObjInstance);
	}
	else if (!strcmp((const char*)inItemRef, "05")) 
	{
		LeArquivoConfig();
	}
}

void LeArquivoConfig()
{
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLError eResult = xmlDoc.LoadFile(pathConfigFile.c_str());
	std::string auxString;
	if (eResult == tinyxml2::XML_SUCCESS)
	{
		auxString.clear();
		auxString = "CIPqdt Master: Arquivo de configuração carregado com sucesso.\n";
		XPLMDebugString(auxString.c_str());
		tinyxml2::XMLElement* pRootElement = xmlDoc.RootElement();
		if (pRootElement != NULL)
		{
			tinyxml2::XMLElement* posicao_objeto_Element = pRootElement->FirstChildElement("posicao_objeto");
			if (posicao_objeto_Element != NULL)
			{
				posicao_objeto_Element->QueryDoubleAttribute("lat", &lat);
				posicao_objeto_Element->QueryDoubleAttribute("lon", &lon);
				posicao_objeto_Element->QueryDoubleAttribute("azimute", &azimute);
				auxString.clear();
				auxString = "CIPqdt Master: lat = " + std::to_string(lat) + "\n";
				auxString += "CIPqdt Master: lon = " + std::to_string(lon) + "\n";
				auxString += "CIPqdt Master: azimute = " + std::to_string(azimute) + "\n";
				XPLMDebugString(auxString.c_str());
			}
			auxString.clear();
			auxString.append("./Resources/plugins/CIPqdt_Master/objetos/");
			tinyxml2::XMLElement* pElement_ObjNormal = pRootElement->FirstChildElement("ObjNormal");
			if (pElement_ObjNormal != NULL)
			{
				auxString.append(pElement_ObjNormal->GetText());
				pathObjNormal.clear();
				pathObjNormal.append(auxString);
				auxString.clear();
				auxString = "CIPqdt Master: pathObjNormal = " + pathObjNormal + "\n";
				XPLMDebugString(auxString.c_str());
			}
			auxString.clear();
			auxString.append("./Resources/plugins/CIPqdt_Master/objetos/");
			tinyxml2::XMLElement* pElement_ObjImpTemp = pRootElement->FirstChildElement("ObjImpTemp");
			if (pElement_ObjImpTemp != NULL)
			{
				auxString.append(pElement_ObjImpTemp->GetText());
				pathObjTemp.clear();
				pathObjTemp.append(auxString);
				auxString.clear();
				auxString = "CIPqdt Master: pathObjTemp = " + pathObjTemp + "\n";
				XPLMDebugString(auxString.c_str());
			}
			auxString.clear();
			auxString.append("./Resources/plugins/CIPqdt_Master/objetos/");
			tinyxml2::XMLElement* pElement_ObjImpDef = pRootElement->FirstChildElement("ObjImpDef");
			if (pElement_ObjImpDef != NULL)
			{
				auxString.append(pElement_ObjImpDef->GetText());
				pathObjDef.clear();
				pathObjDef.append(auxString);
				auxString.clear();
				auxString = "CIPqdt Master: pathObjDef = " + pathObjDef + "\n";
				XPLMDebugString(auxString.c_str());
			}
		}
		else
		{
			auxString.clear();
			auxString = "CIPqdt Master: Erro TninyXML2: " + std::to_string(eResult);
			XPLMDebugString(auxString.c_str());
			XPLMDebugString("CIPqdt Master: Não foi possível abrir o arquivo config.xml\n");
			XPLMDebugString("CIPqdt Master: XML parser error\n");
		}
	}
}