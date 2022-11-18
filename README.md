# ComputerGraph-Roller Coasters

1. 實現Cardinal Splines與B-spline，基礎程序化生成軌道。  
2. OpenGL 1.0函數(未使用Shader)。  
3. obj模型載入與基本動畫與動畫路徑控制。  
4. 軌道使用Adaptive subdivision適當減少模型面。  

## Libraries
 1. [fltk-1.3.2](https://www.fltk.org/)
 2. OpenGL 1.0
 3. [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
 
## Models
 1. [Voxel Trees](https://skfb.ly/6FURN)
 
## 功能說明
<img src="https://i.imgur.com/aTm1lxO.png" width="417" height="329" />


1. Run控制動畫是否開始/暫停。  
2. 控制車頭立即向前移動。  
3. 控制車頭立即向後移動。  
4. 車頭移動根據實際距離/節點數量，關閉時無法開啟Physics功能。  
5. 車頭移動速度，Physics功能開啟時為車頭推力大小。  
6. 任意旋轉視角。  
7. 視角隨車頭移動。  
8. 俯視角。  
9. 選擇軌道內插方式。  
10. Adaptive subdivision 開啟/關閉。  
11. 顯示軌道線段，便於觀察Adaptive subdivision效果。  
12. 新增軌道路徑的控制節點。  
13. 刪除軌道路徑的控制節點。  
14. 載入txt檔軌道路徑  
15. 儲存目前軌道路徑。  
16. 恢復開啟程式時的軌道。  
17. 調整cardinal cubic spline的參數Tension。  
18. 旋轉控制節點。  
19. 控制車廂數量。  
20. 車頭頭燈開啟/關閉。  
21. 物理模擬開啟/關閉，當ArcLength關閉時無法開啟。  
22. 車頭產生煙霧開啟/關閉。  
23. 陽光光源角度θ。  
24. 陽光光源角度φ。  


## 運行結果
<img src="https://i.imgur.com/VvQptTi.png" width="400" height="316" /><img src="https://i.imgur.com/b9ftiz1.png" width="400" height="316" />
<img src="https://i.imgur.com/ki1jXtg.png" width="400" height="316" /><img src="https://i.imgur.com/Xq1iVzA.png" width="400" height="316" />
