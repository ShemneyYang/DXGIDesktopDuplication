#! /usr/bin/env python
# -*- coding: utf-8 -*-

import subprocess
import os
import sys
import string
import _winreg
import copy

def _getVCProjectFile():
	dirs = os.listdir( "./" )

	for file in dirs:
		print os.path.realpath(file)	
		
def _replaceLibYuvConfiguration(Configuration_x64):
	Configuration_x64.set('Name', 'Release|x64')
	Configuration_x64.set('IntermediateDirectory', '$(PlatformName)\$(ConfigurationName)')
	Configuration_x64.set('OutputDirectory', '$(PlatformName)\$(ConfigurationName)')
	
	for toolNode in Configuration_x64.iter('Tool'):
		if (toolNode.attrib.get('Name') == "VCLinkerTool"):
			OutputFile = toolNode.attrib.get('OutputFile').replace("libyuv.dll", "libyuv_x64.dll", 1);
			AdditionalDependencies = toolNode.attrib.get('AdditionalDependencies').replace("jpeg-static.lib", "jpeg-static_x64.lib", 1)
			toolNode.set('AdditionalDependencies', AdditionalDependencies)
			toolNode.set('OutputFile', OutputFile)
			toolNode.set('TargetMachine', '17')

		if (toolNode.attrib.get('Name') == "VCCLCompilerTool"):
			AssemblerListingLocation = toolNode.attrib.get('AssemblerListingLocation').replace("release", "x64\\release", 1);
			ObjectFile = toolNode.attrib.get('ObjectFile').replace("release", "x64\\release", 1);
			toolNode.set('ObjectFile', ObjectFile)
			toolNode.set('AssemblerListingLocation', AssemblerListingLocation)		

	return Configuration_x64;


def main():	
	if (len(sys.argv) <= 1):
		print('no vcproject path input')
		return;
   
	vcrojectFilePath = sys.argv[1]
	print('vcrojectFilePath=%s' % vcrojectFilePath)

	import xml.etree.cElementTree as et
	parser = et.parse(vcrojectFilePath)
	root = parser.getroot()
	root.set('Name', 'libyuv_x64')

	Platforms = root.find('Platforms')
	for neighbor in Platforms.iter('Platform'):
		if (neighbor.attrib.get('Name') == "Win32"):
			Platform_x64 = copy.deepcopy(neighbor)
			Platform_x64.set('Name', 'x64')
			Platforms.append(Platform_x64)
			#print(et.dump(Platforms))

	Configurations = root.find('Configurations')
	for neighbor in Configurations.iter('Configuration'):
		if (neighbor.attrib.get('Name') == "Release|Win32"):
			Configuration_x64 = _replaceLibYuvConfiguration(copy.deepcopy(neighbor))
			Configurations.append(Configuration_x64);
	
	Files = root.find('Files')
	#print(et.dump(Files))
	for filterNode in Files.iter('Filter'):
		if (filterNode.attrib.get('Name') == "Generated Files"):
			Files.remove(filterNode)
			break

	x64ProjectPath = vcrojectFilePath.replace("libyuv.vcproj", "libyuv_x64.vcproj", 1);

	parser.write(x64ProjectPath, "Windows-1252");

	print("make libyuv_x64 succeed!")

if __name__ == '__main__':
    main() 