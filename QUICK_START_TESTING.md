# 🎮 Quick Start - Testing Diablo Mod

## 🚀 Launch Game (Copy-Paste This!)

```powershell
cd C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output
.\ioquake3_diablo.exe +set fs_basepath "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug" +set fs_game my_diablo_output +set sv_pure 0 +set vm_cgame 0 +set vm_ui 0 +set vm_game 0 +set developer 1 +set logfile 2 +map my_level
```

---

## ⌨️ Essential Keys

### Movement
- **W** - Move forward
- **S** - Move backward
- **A** - Strafe left
- **D** - Strafe right
- **Space** - Fly up
- **Ctrl** - Fly down
- **Mouse** - Look around

### Console
- **`~`** (tilde) - Open/close console
- **`** - Alternative console key (US keyboards)

---

## 🔧 Console Commands (Type in game)

### Display Info
```
/cg_drawfps 1          # Show FPS counter (top-right)
/r_showtris 1          # Wireframe mode (see terrain triangles)
/r_speeds 1            # Show render stats
```

### Debug Terrain
```
/r_info                # Renderer information
/gfxinfo               # GPU details
/status                # Server status
```

### Movement Testing
```
/noclip                # Toggle noclip (already on by default)
/god                   # Invulnerability
/give all              # All weapons (for testing)
```

### Shader Reload
```
/vid_restart           # Restart video (reloads shaders)
```

---

## ✅ What to Check

### 1. **Game Launches**
   - ✓ Window appears
   - ✓ No crash
   - ✓ Console accessible (press `~`)

### 2. **Renderer Loaded**
   Open console and look for:
   ```
   Loading "renderer_my_diablo.dll"
   ```
   Should NOT say "renderer_opengl1.dll" or "renderer_opengl2.dll"

### 3. **Terrain Visible**
   - ✓ Green grass texture
   - ✓ No black triangles
   - ✓ Smooth surface
   - ✓ No flickering

### 4. **Performance**
   Type: `/cg_drawfps 1`
   - ✓ FPS > 60 (should be easy on modern PC)
   - ✓ No stuttering
   - ✓ Smooth movement

### 5. **No Errors**
   Check console for:
   - ❌ "max_polys reached" (should NOT appear!)
   - ❌ Red error messages
   - ❌ Warnings about missing files

### 6. **Flight Works**
   - ✓ Press Space - fly up
   - ✓ Press Ctrl - fly down
   - ✓ WASD - move around
   - ✓ Don't fall (gravity disabled)

---

## 🐛 Troubleshooting

### Game won't start
1. Check if all DLLs are present:
   ```powershell
   Get-ChildItem "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output" -Filter "*.dll"
   ```
   Should see:
   - cgame.dll
   - qagame.dll
   - ui.dll
   - renderer_my_diablo.dll
   - renderer_terrain.dll

### Black screen / No terrain
1. Fly UP (hold Space for 3 seconds)
2. Look DOWN (move mouse down)
3. If still nothing, check console for errors

### "max_polys reached" warning
**This should NOT happen!** We increased MAX_POLYS to 8192.
If you see this:
1. Report it immediately
2. Check which renderer loaded (should be renderer_my_diablo.dll)

### Texture missing (white terrain)
1. Check if file exists:
   ```powershell
   Test-Path "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\textures\terrain\base_grass.jpg"
   ```
2. If missing, copy from baseq3/textures/terrain/

### Game crashes
1. Check qconsole.log (if exists)
2. Rebuild DLLs:
   ```powershell
   cd C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3
   cmake --build build --config Debug --target cgame_diablo --target qagame_diablo --target ui_diablo --target renderer_terrain --parallel
   ```

---

## 📸 Screenshots to Take

If possible, take screenshots of:
1. Terrain from above (bird's eye view)
2. Terrain from ground level (close-up texture)
3. Console with FPS counter visible
4. Wireframe mode (`/r_showtris 1`)

---

## 📊 Expected Results

### Performance
- **FPS:** 100-300+ (on modern GPU)
- **Frame time:** < 10ms
- **No stuttering:** Smooth flight

### Visuals
- **Terrain:** Green grass texture, smooth hills
- **Lighting:** Uniform brightness (0.5 gray)
- **No artifacts:** No black triangles, no flickering

### Functionality
- **Movement:** Full 6DOF flight (no collision)
- **Camera:** Smooth mouse look
- **Console:** Responsive, no lag

---

## 🎯 Quick Test (30 seconds)

1. **Launch game** (use command above)
2. **Open console** (press `~`)
3. **Type:** `/cg_drawfps 1`
4. **Fly up** (hold Space for 3 seconds)
5. **Look down** (move mouse down)
6. **Check:**
   - ✓ Terrain visible?
   - ✓ FPS > 60?
   - ✓ No errors in console?

**If all ✓ → SUCCESS! Everything works!**

---

## 📝 Report Back

After testing, tell me:

1. **Did the game launch?** (Yes/No)
2. **Is terrain visible?** (Yes/No)
3. **What FPS do you get?** (number)
4. **Any errors in console?** (copy/paste)
5. **Does noclip work?** (can you fly?)

---

## 🚀 Next Steps After Successful Test

1. ✅ **Terrain works** → Implement collision detection
2. ✅ **Performance good** → Add more optimizations (quadtree, caching)
3. ✅ **No crashes** → Integrate with game logic (player collision)

---

**Good luck!** 🎮✨
