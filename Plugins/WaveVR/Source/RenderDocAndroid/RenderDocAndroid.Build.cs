// Copyright 2020 htc

using System.IO;
using UnrealBuildTool;

public class RenderDocAndroid : ModuleRules
{
	public RenderDocAndroid(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		string ModuleInc = ModuleDirectory + "/include";

		PublicSystemIncludePaths.Add(ModuleInc);

		if (Target.Platform == UnrealTargetPlatform.Android) {
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "RenderDocAndroid_UPL.xml"));
		}
	}
}
