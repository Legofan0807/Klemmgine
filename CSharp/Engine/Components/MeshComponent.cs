﻿using Engine.Native;
using System;
using System.Runtime.InteropServices;
namespace Engine;

/**
 * @brief
 * MeshComponent in C#.
 * 
 * Can be attached to any Engine.SceneObject.
 * 
 * C++ equivalent: MeshComponent.
 * 
 * @ingroup CSharp-Components
 */
public class MeshComponent : ObjectComponent
{
	private delegate IntPtr NewModel([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);


	public MeshComponent()
	{
	}

	/**
	* @brief
	* Loads a mesh file (.jsm) from the given file name. (without path or extension)
	* 
	* @param File
	* The file to load.
	*/
	public void Load(string File)
	{
		if (Parent == null)
		{
			return;
		}
		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), [NativePtr, Parent.NativePtr]);
		}
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMeshComponent", typeof(NewModel), [File, Parent.NativePtr]);
	}

	public override void Tick()
	{
	}
}