# ComputerGraph-Roller Coasters

1. 實現Cardinal Splines與B-spline。  
2. OpenGL 1.0函數(未使用Shader)。  
3. obj模型載入與基本動畫。  
4. 軌道使用Adaptive subdivision適當減少模型面。  

## Libraries
 1. [fltk-1.3.2](https://www.fltk.org/)
 2. OpenGL 1.0
 3. [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
 
## Models
 1. [Voxel Trees](https://skfb.ly/6FURN)
 
## 功能說明
<img src="https://i.imgur.com/aTm1lxO.png" width="417" height="329" />


 > 1. Run控制動畫是否開始/暫停。  <br>
 > 2. 3. 控制車頭立即向前/後移動。  <br>
 > 4. 車頭移動根據實際距離/節點數量，關閉時無法開啟Physics功能。  <br>
 > 5. 車頭移動速度，Physics功能開啟時為車頭推力大小。<br>
 > 6. 7. 8. 觀看角度，分別為任意移動、隨車頭移動、俯視。<br>
 > 9. 選擇軌道內插方式。<br>
 > 10. Adaptive subdivision 開啟/關閉。<br>
 > 11. 顯示軌道線段，便於觀察Adaptive subdivision效果。<br>
 > 12. 13. 新增/刪除軌道路徑的控制節點。<br>
 > 14. 15. 載入txt檔軌道路徑、儲存目前軌道路徑。<br>
 > 16. 恢復開啟程式時的軌道。<br>
 > 17. 調整cardinal cubic spline的參數Tension。<br>
 > 18. 旋轉控制節點。<br>
 > 19. 控制車廂數量。<br>
 > 20. 車頭頭燈開啟/關閉。<br>
 > 21. 物理模擬開啟/關閉，當ArcLength關閉時無法開啟。<br>
 > 22. 車頭產生煙霧開啟/關閉。<br>
 > 23. 24. 陽光光源位置(球坐標系)。<br>


## 運行結果
<img src="https://i.imgur.com/VvQptTi.png" width="400" height="316" /><img src="https://i.imgur.com/b9ftiz1.png" width="400" height="316" />
<img src="https://i.imgur.com/ki1jXtg.png" width="400" height="316" /><img src="https://i.imgur.com/Xq1iVzA.png" width="400" height="316" />
