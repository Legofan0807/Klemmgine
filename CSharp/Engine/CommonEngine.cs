﻿using System.Runtime.InteropServices;
using System;

/**
 * @defgroup CSharp
 * 
 * @brief
 * C# engine API.
 * 
 * Most C# classes have a C++ equivalent class/struct/namespace.
 * If there is an equivalent in C++, the differences between the C# version will be noted.
 */

/**
 * @ingroup CSharp
 * @brief
 * (C#) Engine namespace. Contains C# engine functions.
 * 
 * `Engine` is the namespace that contains all C# functions that belong to the engine.
 */
namespace Engine
{
	public static class CameraShake
	{
		private delegate void CameraShakeDelegate(float i);

		public static void PlayCameraShake(float Intensity)
		{
			NativeFunction.CallNativeFunction("PlayDefaultCameraShake", typeof(CameraShakeDelegate), new object[] { Intensity });
		}
	}

	/**
	 * @brief
	 * Contains functions related to the engine console.
	 * 
	 * Equivalent to Console:: namespace in <Engine/Console.h>. It needed to be
	 * renamed because it's original name conflicted with the System.Console namespace.
	 */
	public static class Command
	{
		internal delegate bool CommandDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string Command);

		/**
		 * @brief
		 * Executes a console command.
		 */
		public static bool ExecuteConsoleCommand(string Command)
		{
			return (bool)NativeFunction.CallNativeFunction("CallConsoleCommand", typeof(CommandDelegate), new object[] { Command });
		}
	}

	public static class Collision
	{
		private delegate HitResponse LineTraceDelegate(Vector3 Start, Vector3 End, IntPtr This);

		[StructLayout(LayoutKind.Sequential)]
		public struct HitResponse
		{
			[MarshalAs(UnmanagedType.U1)]
			public bool Hit;
			public Vector3 ImpactPoint;
			public float t;
			public IntPtr HitObject;
			public IntPtr HitComponent;
			public Vector3 Normal;
		}

		public static HitResponse LineTrace(Vector3 Start, Vector3 End, WorldObject This)
		{
			return (HitResponse)NativeFunction.CallNativeFunction("NativeRaycast", typeof(LineTraceDelegate), new object[] { Start, End, This.NativeObject });
		}
	}

	public static class Stats
	{
		/// DeltaTime. Time between the last frame and the one before.
		public static float DeltaTime = 0.0f;
		/// True if in editor, false if not.
		public static bool InEditor = false;
	}


	/**
	 * @brief
	 * Class containing functions related to scenes.
	 * 
	 * C++ equivalent: Scene namespace.
	 * 
	 * @ingroup CSharp
	 */
	public static class Scene
	{
		private delegate void LoadSceneDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SceneName);
		/**
		 * @brief
		 * Loads a new scene with from the given name.
		 * 
		 * C++ equivalent: Scene::LoadNewScene
		 * 
		 * @param SceneName
		 * The name of the scene file, without a path or extension.
		 */
		public static void LoadScene(string SceneName)
		{
			NativeFunction.CallNativeFunction("LoadScene", typeof(LoadSceneDelegate), new object[] { SceneName });
		}
	}

	public static class Sound
	{
		private delegate IntPtr LoadSoundDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string SoundName);
		private delegate void UnloadSoundDelegate(IntPtr SoundBuffer);
		private delegate void PlaySound(IntPtr s, float Pitch, float Volume, bool Looping);

		public class SoundBuffer
		{
			readonly IntPtr BufferPtr = new();
			public SoundBuffer(IntPtr Buffer)
			{
				BufferPtr = Buffer;
			}

			public void Play(float Pitch = 1, float Volume = 1, bool Looping = false)
			{
				NativeFunction.CallNativeFunction("PlaySound", typeof(PlaySound), new object[] { BufferPtr, Pitch, Volume, Looping });
			}
			~SoundBuffer()
			{
				NativeFunction.CallNativeFunction("UnloadSound", typeof(UnloadSoundDelegate), new object[] { BufferPtr });
			}
		}

		public static SoundBuffer LoadSound(string File)
		{
			IntPtr BufferPtr = (IntPtr)NativeFunction.CallNativeFunction("LoadSound", typeof(LoadSoundDelegate), new object[] { File });
			return new SoundBuffer(BufferPtr);
		}
	}
}