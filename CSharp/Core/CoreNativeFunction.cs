﻿using System;
using System.Runtime.InteropServices;
#nullable enable

namespace Engine.Core;

public static class CoreNativeFunction
{
	static readonly Dictionary<string, IntPtr> LoadedNativeFunctions = [];

	public static void RegisterNativeFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr FunctionPtr)
	{
		if (Engine.GameAssembly == null)
		{
			return;
		}
		Engine.LoadTypeFromAssembly("Engine.Native.NativeFunction")!.GetMethod("RegisterNativeFunction")!.Invoke(null, [Name, FunctionPtr]);
		LoadedNativeFunctions.Add(Name, FunctionPtr);
	}

	public static object? CallNativeFunction(string Name, Type del, object?[]? Args)
	{
		var NewDel = Marshal.GetDelegateForFunctionPointer(LoadedNativeFunctions[Name], del);

		if (NewDel == null)
		{
			return null;
		}

		return NewDel.DynamicInvoke(Args);
	}
	public static void UnloadNativeFunctions()
	{
		LoadedNativeFunctions.Clear();
	}

}
