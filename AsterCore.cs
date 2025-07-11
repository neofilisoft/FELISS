using System;
using AsterCoreWrapper;

class PhysicsEngine : IDisposable
{
    private PhysicsWorld world;
    private RigidBody ground;
    private RigidBody sphere;
    private RigidBody cube;  
    private GPUSolver gpuSolver;

    public PhysicsEngine()
    {
        
        world = new PhysicsWorld();
        world.Gravity = new Vector3(0, -9.8f, 0);
        
    
        ground = world.CreateRigidBody();
        ground.Mass = 0; 
        ground.Position = new Vector3(0, -2, 0);
        ground.SetCollisionShapeAsBox(10.0f, 1.0f, 10.0f);
        
        
        sphere = world.CreateRigidBody();
        sphere.Mass = 1.0f;
        sphere.Restitution = 0.8f;
        sphere.Position = new Vector3(0, 5, 0);
        sphere.SetCollisionShapeAsSphere(1.0f);

        
        cube = world.CreateRigidBody();
        cube.Mass = 2.0f;
        cube.Restitution = 0.5f;
        cube.Position = new Vector3(2, 8, 0);
        cube.SetCollisionShapeAsBox(1.0f, 1.0f, 1.0f);
        
        
        gpuSolver = new GPUSolver(world);
    }

    public void RunSimulation(int frames)
    {
        Console.WriteLine("Starting advanced physics simulation...");
        Console.WriteLine($"Initial positions:");
        Console.WriteLine($"- Sphere: {sphere.Position}");
        Console.WriteLine($"- Cube: {cube.Position}");
        
        for (int i = 0; i < frames; i++)
        {
            world.StepSimulation(0.016f); 
            
            
            if (i % 15 == 0)
            {
                Console.WriteLine($"\nFrame {i}:");
                Console.WriteLine($"Sphere: {sphere.Position} | Velocity: {sphere.LinearVelocity.Magnitude():F2} m/s");
                Console.WriteLine($"Cube: {cube.Position} | Velocity: {cube.LinearVelocity.Magnitude():F2} m/s");
            }
            
            
            if (i % 10 == 0)
            {
                gpuSolver.UploadBodies();
                gpuSolver.SolveCollisionsGPU();
            }

            
            if (i % 20 == 0 && i > 0)
            {
                cube.AddForce(new Vector3(
                    (float)(new Random().NextDouble() * 5 - 2.5f),
                    0,
                    (float)(new Random().NextDouble() * 5 - 2.5f)
                ));
            }
        }
        
        Console.WriteLine("\nSimulation completed!");
        Console.WriteLine($"Final positions:");
        Console.WriteLine($"- Sphere: {sphere.Position}");
        Console.WriteLine($"- Cube: {cube.Position}");
    }

    public void Dispose()
    {
        ground?.Dispose();
        sphere?.Dispose();
        cube?.Dispose();  
        gpuSolver?.Dispose();
        world?.Dispose();
    }

    static void Main()
    {
        using (var engine = new PhysicsEngine())
        {
            engine.RunSimulation(180);  
        }
    }
}


public static class PhysicsExtensions
{
    public static void AddForce(this RigidBody body, float x, float y, float z)
    {
        body.AddForce(new Vector3(x, y, z));
    }
    
    public static void SetPosition(this RigidBody body, float x, float y, float z)
    {
        body.Position = new Vector3(x, y, z);
    }

    
    public static string ToString(this Vector3 vec)
    {
        return $"({vec.X:F2}, {vec.Y:F2}, {vec.Z:F2})";
    }
}