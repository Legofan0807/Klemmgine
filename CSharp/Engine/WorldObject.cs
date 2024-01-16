﻿using Engine;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
#nullable enable

namespace Engine;

/**
 * @defgroup CSharp-Objects
 * @ingroup CSharp
 * 
 * @brief
 * Functions/classes related to objects in C#
 */

/**
 * @brief
 * C# class representing an object.
 * 
 * C++ equivalent: WorldObject class.
 * 
 * @ingroup CSharp-Objects
 * @todo Implement networking functions in C#.
 */
public abstract class WorldObject
{
	/**
	 * @brief
	 * An Attribute that adds the field to the "Objects" menu.
	 * 
	 * @ingroup CSharp-Objects
	 */
	[AttributeUsage(AttributeTargets.Field)]
	public class EditorProperty : Attribute
	{
		public EditorProperty()
		{
			Category = "";
		}
		public EditorProperty(string Category)
		{
			this.Category = Category;
		}
		public string Category;
	}


	public IntPtr NativePtr = new();

	private delegate void SetObjNameDelegate(IntPtr ObjPtr, [MarshalAs(UnmanagedType.LPUTF8Str)] string NewName);

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	private delegate string GetObjNameDelegate(IntPtr ObjPtr);

	private delegate Int32 NewCSObjectDelegate(string TypeName, Transform t);
	private delegate Int32 DestroyObjectDelegate(IntPtr ObjPtr);
	private delegate void SetTransformDelegate(Transform NewTransform, IntPtr NativeObjectPtr);
	private delegate Transform GetTransformDelegate(IntPtr NativeObjectPtr);

	private static Delegate? GetCSObjectDelegate;
	private static Delegate? GetCSObjectByPtrDelegate;

	private static readonly Dictionary<IntPtr, WorldObject> Objects = [];

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public string GetEditorProperties()
	{
		var ObjectType = GetType();
		string PropertyString = "";
		foreach (var i in ObjectType.GetFields(BindingFlags.NonPublic | BindingFlags.Instance))
		{
			if (!i.IsDefined(typeof(EditorProperty)))
			{
				continue;
			}

			var attr = i.GetCustomAttribute<EditorProperty>()!;

			if (attr.Category.Length > 0)
			{
				PropertyString += i.FieldType.Name + " " + attr.Category + ":" + i.Name + ";";
			}
			else
			{
				PropertyString += i.FieldType.Name + " " + GetType().Name + ":" + i.Name + ";";
			}
		}

		return PropertyString;
	}

	/**
	 * @brief
	 * Returns a C# object corrisponding to the given native object.
	 * 
	 * If no object has been found, a new Engine.NativePtr is created from the given pointer.
	 */
	public static WorldObject? GetObjectFromNativePointer(IntPtr Pointer)
	{
		WorldObject? ManagedObject = (WorldObject?)GetCSObjectByPtrDelegate?.DynamicInvoke(Pointer);
		if (ManagedObject != null)
		{
			return ManagedObject;
		}

		var NewObject = new NativeObject();
		NewObject.LoadFromPtr(Pointer);
		return NewObject;
	}

	/**
	 * @brief
	 * Returns the type of this object's the native object.
	 * 
	 * If this is a managed object, this will always be "CSharpObject".
	 * If this object has the type Engine.NativePtr, this will be the name of the class this object represents.
	 */
	public string GetNativeTypeName()
	{
		return (string)NativeFunction.CallNativeFunction("GetTypeNameOfObject", typeof(GetObjNameDelegate), new object[] { NativePtr })!;
	}

	/**
	 * @brief
	 * Spawns a new CSharpObject with the given type and transform.
	 * 
	 * @param TypeName
	 * The name of the type that the new object should have.
	 * 
	 * @param t
	 * The Transform of the new object.
	 */
	public static WorldObject? NewCSObject(string TypeName, Transform t)
	{
		object? RetVal = NativeFunction.CallNativeFunction("NewCSObject", typeof(NewCSObjectDelegate), new object[] { TypeName, t });
		if (RetVal == null)
		{
			return null;
		}
		Int32 ObjID = (Int32)RetVal;
		if (GetCSObjectDelegate == null)
		{
			return null;
		}

		object? objRef = GetCSObjectDelegate.DynamicInvoke(ObjID);
		if (objRef == null)
		{
			return null;
		}
		return (WorldObject)objRef;
	}

	/**
	 * @brief
	 * Sets the name of the object.
	 */
	public void SetName(string NewName)
	{
		NativeFunction.CallNativeFunction("SetObjectName", typeof(SetObjNameDelegate), new object[] { NativePtr, NewName });
	}

	/**
	 * @brief
	 * Gets the name of the object.
	 */
	public string GetName()
	{
		return (string)NativeFunction.CallNativeFunction("GetObjectName", typeof(GetObjNameDelegate), new object[] { NativePtr })!;
	}


	/**
	 * @brief
	 * Gets the native Transform.Position value of the object.
	 */
	public Vector3 GetPosition()
	{
		return GetTransform().Position;
	}

	/**
	 * @brief
	 * Sets the native Transform.Position value of the object.
	 */
	public void SetPosition(Vector3 NewPosition)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewPosition;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform.Rotation value of the object.
	 */
	public Vector3 GetRotation()
	{
		return GetTransform().Rotation;
	}

	/**
	 * @brief
	 * Sets the native Transform.Rotation value of the object.
	 */
	public void SetRotation(Vector3 NewRotation)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewRotation;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform.Scale value of the object.
	 */
	public Vector3 GetScale()
	{
		return GetTransform().Scale;
	}

	/**
	 * @brief
	 * Sets the native Transform.Scale value of the object.
	 */
	public void SetScale(Vector3 NewScale)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewScale;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform value of the object.
	 */
	public Transform GetTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("GetObjectTransform", typeof(GetTransformDelegate), new object[] { NativePtr })!;
	}
	/**
	 * @brief
	 * Sets the native Transform value of the object.
	 */
	public void SetTransform(Transform NewTransform)
	{
		NativeFunction.CallNativeFunction("SetObjectTransform", typeof(SetTransformDelegate), new object[] { NewTransform, NativePtr });
	}

	readonly List<ObjectComponent> AttachedComponents = [];

	/**
	 * @brief
	 * Destroys the given object.
	 * 
	 * @param o
	 * The object to destroy.
	 */
	public static void DestroyObject(WorldObject o)
	{
		NativeFunction.CallNativeFunction("DestroyObject", typeof(DestroyObjectDelegate), new object[] { o.NativePtr });
		return;
	}

	public static void LoadGetObjectFunctions(Delegate GetCSObjByID, Delegate GetCSObjByPtr)
	{
		GetCSObjectDelegate = GetCSObjByID;
		GetCSObjectByPtrDelegate = GetCSObjByPtr;
	}

	/**
	 * @brief
	 * Attaches the given component to the object.
	 */
	public void Attach(ObjectComponent c)
	{
		c.Parent = this;
		AttachedComponents.Add(c);
		c.OnAttached();
	}

	/**
	 * @brief
	 * Deletes and detaches the given component from the object.
	 */
	public void Detach(ObjectComponent c)
	{
		c.Destroy();
		AttachedComponents.Remove(c);
	}


	public void UpdateComponents()
	{
		foreach (var c in AttachedComponents)
		{
			c.Tick();
		}
	}

	/**
	 * @brief
	 * Function called each frame.
	 * 
	 * Each object has to implement this function.
	 */
	public abstract void Update();
	
	/**
	 * @brief
	 * Function called once the object is created.
	 * 
	 * Each object has to implement this function.
	 */
	public abstract void Begin();
	
	/**
	 * @brief
	 * Function called if the object is destroyed.
	 * 
	 * Each object has to implement this function.
	 */
	protected abstract void Destroy();
	public void DestroyObjectInternal()
	{
		Destroy();
		foreach (var c in AttachedComponents)
		{
			c.Destroy();
		}
		AttachedComponents.Clear();
	}
}