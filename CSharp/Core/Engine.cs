﻿using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;
using System.Xml.Linq;

[StructLayout(LayoutKind.Sequential)]
public struct EngineVector
{
	public float X = 0;
	public float Y = 0;
	public float Z = 0;

	public EngineVector()
	{
	}
	public EngineVector(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
}
[StructLayout(LayoutKind.Sequential)]
public struct EngineTransform
{
	public EngineVector Position;
	public EngineVector Rotation;
	public EngineVector Scale;

	public EngineTransform()
	{
		Position = new EngineVector();
		Rotation = new EngineVector();
		Scale = new EngineVector();
	}
	public EngineTransform(EngineVector x, EngineVector y, EngineVector z)
	{
		Position = x;
		Rotation = y;
		Scale = z;
	}
}

static class Engine
{
	public static Assembly? LoadedAsm = null;
	static Int64 CurrentObjectIndex = 0;
	static Type? StatsObject = null;
	static Type? InputObject = null;

	static readonly Dictionary<Int32, Object> WorldObjects = [];
	static readonly List<Type> WorldObjectTypes = [];

	public static object? GetObjectFromID(Int32 Id)
	{
		if (WorldObjects.TryGetValue(Id, out object? value))
		{
			return value;
		}
		return null;
	}
	static bool LoadedEngine = false;

	public static void LoadAssembly([MarshalAs(UnmanagedType.LPUTF8Str)] string Path, [MarshalAs(UnmanagedType.LPUTF8Str)] string EngineDllPath, bool InEditor)
	{
		WorldObjectTypes.Clear();
		CoreNativeFunction.UnloadNativeFunctions();
		EngineLog.Print("Loading C# assembly...");

		if (!LoadedEngine)
		{
			Assembly.LoadFrom(EngineDllPath);
			LoadedEngine = true;
		}
		else
		{
			LoadTypeFromAssembly("NativeFunction")?.GetMethod("UnloadAll")?.Invoke(null, null);
		}
		LoadedAsm = Assembly.Load(File.ReadAllBytes(Path));
		LoadTypeFromAssembly("Engine.Log")!.GetMethod("LoadLogFunction")!.Invoke(null, [new Action<string, int>(EngineLog.Print)]);


		var WorldObjectType = LoadTypeFromAssembly("Engine.WorldObject");

		if (WorldObjectType == null)
		{
			EngineLog.Print("Could not load WorldObject class.");
			return;
		}

		WorldObjectType?.GetMethod("LoadGetObjectFunction")!.Invoke(null, new object[] { (object)GetObjectFromID });

		foreach (var i in LoadedAsm.GetTypes())
		{
			if (i.IsSubclassOf(WorldObjectType!))
			{
				WorldObjectTypes.Add(i);
			}
		}

		StatsObject = LoadTypeFromAssembly("Engine.Stats");
		StatsObject!.GetField("InEditor")!.SetValue(null, InEditor);
		InputObject = LoadTypeFromAssembly("Engine.Input");
	}

	public static Int32 Instantiate(string obj, EngineTransform t, IntPtr NativeObject)
	{
		foreach (var ObjectType in WorldObjectTypes)
		{
			if (ObjectType.Name == obj)
			{
				object NewObject = Activator.CreateInstance(ObjectType)!;
				if (NewObject == null)
				{
					return 0;
				}

				CurrentObjectIndex++;
				Int32 HashCode = CurrentObjectIndex.GetHashCode();
				if (HashCode == 0)
				{
					HashCode = -1;
				}
				Set(ref NewObject!, "NativeObject", NativeObject);

				WorldObjects.Add(HashCode, NewObject);
				SetVectorFieldOfObject(HashCode, "Position", t.Position);
				SetVectorFieldOfObject(HashCode, "Rotation", t.Rotation);
				SetVectorFieldOfObject(HashCode, "Scale", t.Scale);
				return HashCode;
			}
		}
		return 0;
	}

	public static void Destroy(Int32 ID)
	{
		if (!WorldObjects.ContainsKey(ID))
		{
			return;
		}

		ExecuteFunctionOnObject(ID, "DestroyObjectInternal");
		WorldObjects.Remove(ID);
	}

	public static Type? LoadTypeFromAssembly(string Type)
	{
		if (LoadedAsm == null)
		{
			EngineLog.Print("Tried to call method while the C# assembly is unloaded!", 2);
			return null;
		}

		foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies().Reverse())
		{
			var tt = assembly.GetType(Type);
			if (tt != null)
			{
				return tt;
			}
		}
		EngineLog.Print("Failed to load type " + Type, 2);
		return null;
	}


	public static void ExecuteFunctionOnObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string FunctionName)
	{
		if (!WorldObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried to call {0} on the object with ID {1} but that object doesn't exist!", FunctionName, ID));
			return;
		}
		var obj = value;
		var func = obj.GetType().GetMethod(FunctionName);
		if (func == null)
		{
			EngineLog.Print(string.Format("Tried to call {0} on {1} but that function doesn't exist on this class!", FunctionName, obj.GetType().Name));
			return;
		}
		func.Invoke(obj, []);
	}

	public static object? Get(object obj, string member)
	{
		var memb = obj.GetType().GetField(member);
		return memb?.GetValue(obj);
	}
	public static void Set(ref object? obj, string field, object value)
	{
		var memb = obj?.GetType().GetField(field);
		memb?.SetValueDirect(__makeref(obj), value);
	}

	public static EngineVector GetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field)
	{
		if (!WorldObjects.TryGetValue(ID, out object? value))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
			return new EngineVector();
		}
		var obj = value;
		var pos = Get(obj, Field);

		return new EngineVector((float)Get(pos!, "X")!, (float)Get(pos!, "Y")!, (float)Get(pos!, "Z")!);
	}

	public static void SetVectorFieldOfObject(Int32 ID, [MarshalAs(UnmanagedType.LPUTF8Str)] string Field, EngineVector NewValue)
	{
		if (!WorldObjects.ContainsKey(ID))
		{
			EngineLog.Print(string.Format("Tried to access {1} of object with ID {0} but that object doesn't exist!", ID, Field));
		}
		var obj = WorldObjects[ID];
		var pos = obj?.GetType()?.GetField(Field)?.GetValue(obj);
		Set(ref pos, "X", NewValue.X);
		Set(ref pos, "Y", NewValue.Y);
		Set(ref pos, "Z", NewValue.Z);
		obj?.GetType()?.GetField(Field)?.SetValue(obj, pos);
	}

	public static void SetDelta(float NewDelta)
	{
		StatsObject!.GetField("DeltaTime")?.SetValue(null, NewDelta);
		InputObject!.GetMethod("UpdateGamepadList")?.Invoke(null, null);
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string GetAllObjectTypes()
	{
		string AllTypes = "";
		foreach (var i in WorldObjectTypes)
		{
			if (i.Namespace != null)
			{
				AllTypes += i.Namespace.Replace('.', '/') + "/" + i.Name + " ";
			}
			else
			{
				AllTypes += i.Name + " ";
			}
		}

		return AllTypes;
	}

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public static string GetStartupSceneInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return "";
		}
		var StartupScene = ProjectType.GetMethod("GetStartupScene");
		if (StartupScene == null)
		{
			EngineLog.Print("Failed to load Project.GetStartupScene()", 2);
			return "";
		}
		return (string)StartupScene.Invoke(null, null)!;
	}

	public static void OnLaunchInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return;
		}
		var OnLaunch = ProjectType.GetMethod("OnLaunch");
		if (OnLaunch == null)
		{
			EngineLog.Print("Failed to load Project.OnLaunch()", 2);
			return;
		}
		OnLaunch.Invoke(null, null);
		return;
	}

	public static string GetNameInternally()
	{
		var ProjectType = LoadTypeFromAssembly("Project");
		if (ProjectType == null)
		{
			EngineLog.Print("Failed to load project type", 2);
			return "";
		}
		var GetProjectName = ProjectType.GetMethod("GetProjectName");
		if (GetProjectName == null)
		{
			EngineLog.Print("Failed to load Project.GetProjectName()", 2);
			return "";
		}
		return (string)GetProjectName.Invoke(null, null)!;
	}

}