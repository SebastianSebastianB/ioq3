# 🎮 Game Test Checklist - Diablo Mod

## 🚀 Launch Command
```powershell
cd C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output
.\ioquake3_diablo.exe +set fs_basepath "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug" +set fs_game my_diablo_output +set sv_pure 0 +set vm_cgame 0 +set vm_ui 0 +set vm_game 0 +set developer 1 +set logfile 2 +map my_level
```

---

## ✅ Test 1: Game Launch
- [ ] Game window opens without crash
- [ ] Main menu loads
- [ ] Console is accessible (press `~` key)
- [ ] No immediate error messages

**How to check:**
1. Game window appears
2. Press `~` to open console
3. Look for red error messages

**Expected result:** Clean startup, no errors

---

## ✅ Test 2: Renderer Loading
- [ ] `renderer_my_diablo.dll` loaded (check console)
- [ ] No "failed to load renderer" errors
- [ ] OpenGL context initialized

**How to check:**
Open console (`~`) and look for:
```
----- Initializing Renderer ----
Loading "renderer_my_diablo.dll"
...loaded renderer_my_diablo
```

**Expected result:** Custom renderer loaded successfully

---

## ✅ Test 3: Map Loading
- [ ] Map `my_level` loads without error
- [ ] Spawn point exists
- [ ] Player spawns in world

**How to check:**
Console should show:
```
Loading map: my_level
...loading my_level.world
...map loaded
```

**Expected result:** Map loads, player spawns

---

## ✅ Test 4: Terrain Rendering
- [ ] Terrain geometry visible
- [ ] Texture applied (base_grass.jpg)
- [ ] No "max_polys reached" warning
- [ ] No black triangles
- [ ] No white/untextured areas

**How to check:**
1. Look around with mouse
2. Open console - check for warnings
3. Use `/r_showtris 1` to see wireframe

**Expected result:** Smooth terrain with grass texture

---

## ✅ Test 5: Player Movement (Noclip Flight)
- [ ] Player can move forward (W key)
- [ ] Player can move backward (S key)
- [ ] Player can strafe (A/D keys)
- [ ] Player can fly up/down (jump/crouch)
- [ ] Player doesn't fall (gravity disabled)
- [ ] No collision with terrain (noclip mode)

**How to check:**
1. Use WASD keys to move
2. Use Space/Ctrl to fly up/down
3. Fly through terrain - should pass through

**Expected result:** Free flight mode, no gravity, no collision

---

## ✅ Test 6: Performance
- [ ] FPS counter shows >60 FPS
- [ ] No stuttering during flight
- [ ] Smooth terrain LOD transitions
- [ ] No crashes after 30 seconds of flight

**How to check:**
1. Enable FPS: `/cg_drawfps 1`
2. Fly around terrain for 30 seconds
3. Check FPS in top-right corner

**Expected result:** Smooth 60+ FPS, no crashes

---

## ✅ Test 7: Console Debug Info
- [ ] No red error messages
- [ ] Custom renderer active
- [ ] DLL modules loaded (cgame, qagame, ui)

**How to check:**
In console, type:
```
/status
/version
/r_info
```

**Expected result:** All systems operational

---

## ✅ Test 8: Texture Tiling
- [ ] Grass texture repeats correctly
- [ ] No stretching or distortion
- [ ] Detail level appropriate (not too blurry)

**How to check:**
1. Fly close to terrain
2. Look at texture detail
3. Fly far away - texture should tile

**Expected result:** Crisp 4x tiled texture

---

## 🔍 Debugging Commands

### View Terrain Wireframe
```
/r_showtris 1     # Show triangle edges (white lines)
/r_showtris 0     # Hide wireframe
```

### Performance Monitoring
```
/cg_drawfps 1     # Show FPS counter
/r_speeds 1       # Show render statistics
/timegraph 1      # Show frame time graph
```

### Renderer Info
```
/r_info           # OpenGL info
/gfxinfo          # Graphics card info
```

### Movement Testing
```
/noclip           # Toggle noclip (should already be on)
/god              # Toggle god mode
/notarget         # Enemies ignore player
```

### Debug Logging
```
/developer 1      # Enable developer messages
/logfile 2        # Log to qconsole.log
```

---

## 🐛 Common Issues & Solutions

### Issue: "max_polys reached" warning
**Solution:** Already fixed - custom renderer has MAX_POLYS=8192

### Issue: Black triangles on terrain
**Solution:** Already fixed - winding order corrected to CW

### Issue: White/untextured terrain
**Solution:** Check if base_grass.jpg exists in textures/terrain/

### Issue: Player falls through terrain
**Solution:** Expected! Noclip is enabled for testing

### Issue: Game crashes on startup
**Solution:** 
1. Check if all DLLs are in my_diablo_output/
2. Verify renderer_my_diablo.dll exists
3. Check qconsole.log for errors

### Issue: Terrain not visible
**Solution:**
1. Fly upward (Space key)
2. Look down
3. Check if my_level.world loaded

---

## 📊 Test Results

### Last Test: [DATE/TIME]
- [ ] All tests passed
- [ ] Some issues found (see below)
- [ ] Critical bugs blocking gameplay

**Issues found:**
1. 
2. 
3. 

**FPS:** ___ (average)
**Crashes:** Yes / No
**Terrain visible:** Yes / No
**Noclip working:** Yes / No

---

## 📝 Notes

Use this space to document any observations:

- 
- 
- 

---

## ✅ Sign-off

**Tester:** _______________
**Date:** _______________
**Build version:** Debug (02.10.2025)
**Status:** ☐ Pass  ☐ Fail  ☐ Needs Review
