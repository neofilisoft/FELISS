// =====================================================================
// Feliss Engine — C# Example Script
// assets/scripts/PlayerController.cs
//
// Compile: dotnet build  → produces GameScripts.dll
// Load:    fls_csharp_load_assembly(engine, "GameScripts.dll");
// =====================================================================
using System;

namespace FelissGame
{
    // ---- Base class provided by the engine's managed assembly ----
    // (FelissEngine.dll must be referenced in your .csproj)
    public abstract class BehaviourScript
    {
        public ulong EntityId { get; internal set; }

        public virtual void OnStart()            { }
        public virtual void OnUpdate(float dt)   { }
        public virtual void OnDestroy()          { }
        public virtual void OnCollisionEnter(ulong other) { }
        public virtual void OnTriggerEnter(ulong other)   { }
    }

    // ---- Engine API stubs (replace with P/Invoke bindings) ----
    public static class Log
    {
        public static void Info (string msg) => Console.WriteLine($"[INFO ] {msg}");
        public static void Warn (string msg) => Console.WriteLine($"[WARN ] {msg}");
        public static void Error(string msg) => Console.WriteLine($"[ERROR] {msg}");
    }

    public struct Vec3 { public float X, Y, Z; public Vec3(float x,float y,float z){X=x;Y=y;Z=z;} }

    public static class Transform
    {
        public static void SetPosition(ulong id, Vec3 p) { /* P/Invoke: fls_transform_set_pos */ }
        public static Vec3 GetPosition(ulong id)         { return new Vec3(0,0,0); }
        public static void Translate   (ulong id, Vec3 d) { }
    }

    public static class Input
    {
        public static bool GetKey(int keyCode) { return false; /* P/Invoke: fls_key_pressed */ }
        public static class KeyCode
        {
            public const int W = 87; public const int A = 65;
            public const int S = 83; public const int D = 68;
            public const int Space = 32; public const int LeftShift = 340;
        }
    }

    public static class Time
    {
        public static float Delta   { get; internal set; }
        public static float Elapsed { get; internal set; }
    }

    // ====================================================================
    // PlayerController
    // ====================================================================
    public class PlayerController : BehaviourScript
    {
        public float Speed     = 5.0f;
        public float JumpForce = 8.0f;

        private bool  m_grounded   = true;
        private float m_verticalVel = 0.0f;
        private const float Gravity = -9.81f;

        public override void OnStart()
        {
            Log.Info($"PlayerController started on entity {EntityId}");
        }

        public override void OnUpdate(float dt)
        {
            var delta = new Vec3(0, 0, 0);

            if (Input.GetKey(Input.KeyCode.W)) delta.Z += Speed * dt;
            if (Input.GetKey(Input.KeyCode.S)) delta.Z -= Speed * dt;
            if (Input.GetKey(Input.KeyCode.A)) delta.X -= Speed * dt;
            if (Input.GetKey(Input.KeyCode.D)) delta.X += Speed * dt;

            // Sprint
            if (Input.GetKey(Input.KeyCode.LeftShift))
            {
                delta.X *= 2.0f;
                delta.Z *= 2.0f;
            }

            // Jump
            if (Input.GetKey(Input.KeyCode.Space) && m_grounded)
            {
                m_verticalVel = JumpForce;
                m_grounded    = false;
                Log.Info("Jump!");
            }

            // Gravity
            if (!m_grounded)
            {
                m_verticalVel += Gravity * dt;
                delta.Y        = m_verticalVel * dt;
            }

            Transform.Translate(EntityId, delta);

            // Simple ground check (Y <= 0)
            var pos = Transform.GetPosition(EntityId);
            if (pos.Y <= 0.0f && !m_grounded)
            {
                Transform.SetPosition(EntityId, new Vec3(pos.X, 0.0f, pos.Z));
                m_verticalVel = 0.0f;
                m_grounded    = true;
            }
        }

        public override void OnDestroy()
        {
            Log.Info($"PlayerController destroyed on entity {EntityId}");
        }

        public override void OnCollisionEnter(ulong other)
        {
            Log.Info($"Collision with entity {other}");
        }
    }

    // ====================================================================
    // RotateObject — simple rotation demo
    // ====================================================================
    public class RotateObject : BehaviourScript
    {
        public float SpeedDeg = 90.0f; // degrees / second

        public override void OnUpdate(float dt)
        {
            // TODO: call Transform.Rotate once Quaternion API is exposed
        }
    }
}
