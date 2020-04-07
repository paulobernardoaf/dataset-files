




RLAPI void InitWindow(int width, int height, const char *title); 
RLAPI bool WindowShouldClose(void); 
RLAPI void CloseWindow(void); 
RLAPI bool IsWindowReady(void); 
RLAPI bool IsWindowMinimized(void); 
RLAPI bool IsWindowResized(void); 
RLAPI bool IsWindowHidden(void); 
RLAPI void ToggleFullscreen(void); 
RLAPI void UnhideWindow(void); 
RLAPI void HideWindow(void); 
RLAPI void SetWindowIcon(Image image); 
RLAPI void SetWindowTitle(const char *title); 
RLAPI void SetWindowPosition(int x, int y); 
RLAPI void SetWindowMonitor(int monitor); 
RLAPI void SetWindowMinSize(int width, int height); 
RLAPI void SetWindowSize(int width, int height); 
RLAPI void *GetWindowHandle(void); 
RLAPI int GetScreenWidth(void); 
RLAPI int GetScreenHeight(void); 
RLAPI int GetMonitorCount(void); 
RLAPI int GetMonitorWidth(int monitor); 
RLAPI int GetMonitorHeight(int monitor); 
RLAPI int GetMonitorPhysicalWidth(int monitor); 
RLAPI int GetMonitorPhysicalHeight(int monitor); 
RLAPI Vector2 GetWindowPosition(void); 
RLAPI const char *GetMonitorName(int monitor); 
RLAPI const char *GetClipboardText(void); 
RLAPI void SetClipboardText(const char *text); 


RLAPI void ShowCursor(void); 
RLAPI void HideCursor(void); 
RLAPI bool IsCursorHidden(void); 
RLAPI void EnableCursor(void); 
RLAPI void DisableCursor(void); 


RLAPI void ClearBackground(Color color); 
RLAPI void BeginDrawing(void); 
RLAPI void EndDrawing(void); 
RLAPI void BeginMode2D(Camera2D camera); 
RLAPI void EndMode2D(void); 
RLAPI void BeginMode3D(Camera3D camera); 
RLAPI void EndMode3D(void); 
RLAPI void BeginTextureMode(RenderTexture2D target); 
RLAPI void EndTextureMode(void); 
RLAPI void BeginScissorMode(int x, int y, int width, int height); 
RLAPI void EndScissorMode(void); 


RLAPI Ray GetMouseRay(Vector2 mousePosition, Camera camera); 
RLAPI Matrix GetCameraMatrix(Camera camera); 
RLAPI Matrix GetCameraMatrix2D(Camera2D camera); 
RLAPI Vector2 GetWorldToScreen(Vector3 position, Camera camera); 
RLAPI Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height); 
RLAPI Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera); 
RLAPI Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera); 


RLAPI void SetTargetFPS(int fps); 
RLAPI int GetFPS(void); 
RLAPI float GetFrameTime(void); 
RLAPI double GetTime(void); 


RLAPI int ColorToInt(Color color); 
RLAPI Vector4 ColorNormalize(Color color); 
RLAPI Color ColorFromNormalized(Vector4 normalized); 
RLAPI Vector3 ColorToHSV(Color color); 
RLAPI Color ColorFromHSV(Vector3 hsv); 
RLAPI Color GetColor(int hexValue); 
RLAPI Color Fade(Color color, float alpha); 


RLAPI void SetConfigFlags(unsigned int flags); 
RLAPI void SetTraceLogLevel(int logType); 
RLAPI void SetTraceLogExit(int logType); 
RLAPI void SetTraceLogCallback(TraceLogCallback callback); 
RLAPI void TraceLog(int logType, const char *text, ...); 
RLAPI void TakeScreenshot(const char *fileName); 
RLAPI int GetRandomValue(int min, int max); 


RLAPI bool FileExists(const char *fileName); 
RLAPI bool IsFileExtension(const char *fileName, const char *ext);
RLAPI bool DirectoryExists(const char *dirPath); 
RLAPI const char *GetExtension(const char *fileName); 
RLAPI const char *GetFileName(const char *filePath); 
RLAPI const char *GetFileNameWithoutExt(const char *filePath); 
RLAPI const char *GetDirectoryPath(const char *filePath); 
RLAPI const char *GetPrevDirectoryPath(const char *dirPath); 
RLAPI const char *GetWorkingDirectory(void); 
RLAPI char **GetDirectoryFiles(const char *dirPath, int *count); 
RLAPI void ClearDirectoryFiles(void); 
RLAPI bool ChangeDirectory(const char *dir); 
RLAPI bool IsFileDropped(void); 
RLAPI char **GetDroppedFiles(int *count); 
RLAPI void ClearDroppedFiles(void); 
RLAPI long GetFileModTime(const char *fileName); 

RLAPI unsigned char *CompressData(unsigned char *data, int dataLength, int *compDataLength); 
RLAPI unsigned char *DecompressData(unsigned char *compData, int compDataLength, int *dataLength); 


RLAPI void StorageSaveValue(int position, int value); 
RLAPI int StorageLoadValue(int position); 

RLAPI void OpenURL(const char *url); 






RLAPI bool IsKeyPressed(int key); 
RLAPI bool IsKeyDown(int key); 
RLAPI bool IsKeyReleased(int key); 
RLAPI bool IsKeyUp(int key); 
RLAPI int GetKeyPressed(void); 
RLAPI void SetExitKey(int key); 


RLAPI bool IsGamepadAvailable(int gamepad); 
RLAPI bool IsGamepadName(int gamepad, const char *name); 
RLAPI const char *GetGamepadName(int gamepad); 
RLAPI bool IsGamepadButtonPressed(int gamepad, int button); 
RLAPI bool IsGamepadButtonDown(int gamepad, int button); 
RLAPI bool IsGamepadButtonReleased(int gamepad, int button); 
RLAPI bool IsGamepadButtonUp(int gamepad, int button); 
RLAPI int GetGamepadButtonPressed(void); 
RLAPI int GetGamepadAxisCount(int gamepad); 
RLAPI float GetGamepadAxisMovement(int gamepad, int axis); 


RLAPI bool IsMouseButtonPressed(int button); 
RLAPI bool IsMouseButtonDown(int button); 
RLAPI bool IsMouseButtonReleased(int button); 
RLAPI bool IsMouseButtonUp(int button); 
RLAPI int GetMouseX(void); 
RLAPI int GetMouseY(void); 
RLAPI Vector2 GetMousePosition(void); 
RLAPI void SetMousePosition(int x, int y); 
RLAPI void SetMouseOffset(int offsetX, int offsetY); 
RLAPI void SetMouseScale(float scaleX, float scaleY); 
RLAPI int GetMouseWheelMove(void); 


RLAPI int GetTouchX(void); 
RLAPI int GetTouchY(void); 
RLAPI Vector2 GetTouchPosition(int index); 




RLAPI void SetGesturesEnabled(unsigned int gestureFlags); 
RLAPI bool IsGestureDetected(int gesture); 
RLAPI int GetGestureDetected(void); 
RLAPI int GetTouchPointsCount(void); 
RLAPI float GetGestureHoldDuration(void); 
RLAPI Vector2 GetGestureDragVector(void); 
RLAPI float GetGestureDragAngle(void); 
RLAPI Vector2 GetGesturePinchVector(void); 
RLAPI float GetGesturePinchAngle(void); 




RLAPI void SetCameraMode(Camera camera, int mode); 
RLAPI void UpdateCamera(Camera *camera); 

RLAPI void SetCameraPanControl(int panKey); 
RLAPI void SetCameraAltControl(int altKey); 
RLAPI void SetCameraSmoothZoomControl(int szKey); 
RLAPI void SetCameraMoveControls(int frontKey, int backKey, int rightKey, int leftKey, int upKey, int downKey); 






RLAPI void DrawPixel(int posX, int posY, Color color); 
RLAPI void DrawPixelV(Vector2 position, Color color); 
RLAPI void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color); 
RLAPI void DrawLineV(Vector2 startPos, Vector2 endPos, Color color); 
RLAPI void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color); 
RLAPI void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color); 
RLAPI void DrawLineStrip(Vector2 *points, int numPoints, Color color); 
RLAPI void DrawCircle(int centerX, int centerY, float radius, Color color); 
RLAPI void DrawCircleSector(Vector2 center, float radius, int startAngle, int endAngle, int segments, Color color); 
RLAPI void DrawCircleSectorLines(Vector2 center, float radius, int startAngle, int endAngle, int segments, Color color); 
RLAPI void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2); 
RLAPI void DrawCircleV(Vector2 center, float radius, Color color); 
RLAPI void DrawCircleLines(int centerX, int centerY, float radius, Color color); 
RLAPI void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color); 
RLAPI void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color); 
RLAPI void DrawRing(Vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, Color color); 
RLAPI void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, Color color); 
RLAPI void DrawRectangle(int posX, int posY, int width, int height, Color color); 
RLAPI void DrawRectangleV(Vector2 position, Vector2 size, Color color); 
RLAPI void DrawRectangleRec(Rectangle rec, Color color); 
RLAPI void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color); 
RLAPI void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2);
RLAPI void DrawRectangleGradientH(int posX, int posY, int width, int height, Color color1, Color color2);
RLAPI void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4); 
RLAPI void DrawRectangleLines(int posX, int posY, int width, int height, Color color); 
RLAPI void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color); 
RLAPI void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color); 
RLAPI void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, int lineThick, Color color); 
RLAPI void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color); 
RLAPI void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color); 
RLAPI void DrawTriangleFan(Vector2 *points, int numPoints, Color color); 
RLAPI void DrawTriangleStrip(Vector2 *points, int pointsCount, Color color); 
RLAPI void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color); 
RLAPI void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color); 


RLAPI bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2); 
RLAPI bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2); 
RLAPI bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec); 
RLAPI Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2); 
RLAPI bool CheckCollisionPointRec(Vector2 point, Rectangle rec); 
RLAPI bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius); 
RLAPI bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3); 






RLAPI Image LoadImage(const char *fileName); 
RLAPI Image LoadImageEx(Color *pixels, int width, int height); 
RLAPI Image LoadImagePro(void *data, int width, int height, int format); 
RLAPI Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize); 
RLAPI void ExportImage(Image image, const char *fileName); 
RLAPI void ExportImageAsCode(Image image, const char *fileName); 
RLAPI Texture2D LoadTexture(const char *fileName); 
RLAPI Texture2D LoadTextureFromImage(Image image); 
RLAPI TextureCubemap LoadTextureCubemap(Image image, int layoutType); 
RLAPI RenderTexture2D LoadRenderTexture(int width, int height); 
RLAPI void UnloadImage(Image image); 
RLAPI void UnloadTexture(Texture2D texture); 
RLAPI void UnloadRenderTexture(RenderTexture2D target); 
RLAPI Color *GetImageData(Image image); 
RLAPI Vector4 *GetImageDataNormalized(Image image); 
RLAPI Rectangle GetImageAlphaBorder(Image image, float threshold); 
RLAPI int GetPixelDataSize(int width, int height, int format); 
RLAPI Image GetTextureData(Texture2D texture); 
RLAPI Image GetScreenData(void); 
RLAPI void UpdateTexture(Texture2D texture, const void *pixels); 


RLAPI Image ImageCopy(Image image); 
RLAPI Image ImageFromImage(Image image, Rectangle rec); 
RLAPI void ImageToPOT(Image *image, Color fillColor); 
RLAPI void ImageFormat(Image *image, int newFormat); 
RLAPI void ImageAlphaMask(Image *image, Image alphaMask); 
RLAPI void ImageAlphaClear(Image *image, Color color, float threshold); 
RLAPI void ImageAlphaCrop(Image *image, float threshold); 
RLAPI void ImageAlphaPremultiply(Image *image); 
RLAPI void ImageCrop(Image *image, Rectangle crop); 
RLAPI void ImageResize(Image *image, int newWidth, int newHeight); 
RLAPI void ImageResizeNN(Image *image, int newWidth,int newHeight); 
RLAPI void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color color); 
RLAPI void ImageMipmaps(Image *image); 
RLAPI void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp); 
RLAPI Color *ImageExtractPalette(Image image, int maxPaletteSize, int *extractCount); 
RLAPI Image ImageText(const char *text, int fontSize, Color color); 
RLAPI Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint); 
RLAPI void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint); 
RLAPI void ImageDrawRectangle(Image *dst, Rectangle rec, Color color); 
RLAPI void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color); 
RLAPI void ImageDrawText(Image *dst, Vector2 position, const char *text, int fontSize, Color color); 
RLAPI void ImageDrawTextEx(Image *dst, Vector2 position, Font font, const char *text, float fontSize, float spacing, Color color); 
RLAPI void ImageFlipVertical(Image *image); 
RLAPI void ImageFlipHorizontal(Image *image); 
RLAPI void ImageRotateCW(Image *image); 
RLAPI void ImageRotateCCW(Image *image); 
RLAPI void ImageColorTint(Image *image, Color color); 
RLAPI void ImageColorInvert(Image *image); 
RLAPI void ImageColorGrayscale(Image *image); 
RLAPI void ImageColorContrast(Image *image, float contrast); 
RLAPI void ImageColorBrightness(Image *image, int brightness); 
RLAPI void ImageColorReplace(Image *image, Color color, Color replace); 


RLAPI Image GenImageColor(int width, int height, Color color); 
RLAPI Image GenImageGradientV(int width, int height, Color top, Color bottom); 
RLAPI Image GenImageGradientH(int width, int height, Color left, Color right); 
RLAPI Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer); 
RLAPI Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2); 
RLAPI Image GenImageWhiteNoise(int width, int height, float factor); 
RLAPI Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale); 
RLAPI Image GenImageCellular(int width, int height, int tileSize); 


RLAPI void GenTextureMipmaps(Texture2D *texture); 
RLAPI void SetTextureFilter(Texture2D texture, int filterMode); 
RLAPI void SetTextureWrap(Texture2D texture, int wrapMode); 


RLAPI void DrawTexture(Texture2D texture, int posX, int posY, Color tint); 
RLAPI void DrawTextureV(Texture2D texture, Vector2 position, Color tint); 
RLAPI void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint); 
RLAPI void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint); 
RLAPI void DrawTextureQuad(Texture2D texture, Vector2 tiling, Vector2 offset, Rectangle quad, Color tint); 
RLAPI void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint); 
RLAPI void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle destRec, Vector2 origin, float rotation, Color tint); 






RLAPI Font GetFontDefault(void); 
RLAPI Font LoadFont(const char *fileName); 
RLAPI Font LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount); 
RLAPI Font LoadFontFromImage(Image image, Color key, int firstChar); 
RLAPI CharInfo *LoadFontData(const char *fileName, int fontSize, int *fontChars, int charsCount, int type); 
RLAPI Image GenImageFontAtlas(const CharInfo *chars, Rectangle **recs, int charsCount, int fontSize, int padding, int packMethod); 
RLAPI void UnloadFont(Font font); 


RLAPI void DrawFPS(int posX, int posY); 
RLAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color); 
RLAPI void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); 
RLAPI void DrawTextRec(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint); 

RLAPI void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float scale, Color tint); 


RLAPI int MeasureText(const char *text, int fontSize); 
RLAPI Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing); 
RLAPI int GetGlyphIndex(Font font, int codepoint); 



RLAPI int TextCopy(char *dst, const char *src); 
RLAPI bool TextIsEqual(const char *text1, const char *text2); 
RLAPI unsigned int TextLength(const char *text); 
RLAPI const char *TextFormat(const char *text, ...); 
RLAPI const char *TextSubtext(const char *text, int position, int length); 
RLAPI char *TextReplace(char *text, const char *replace, const char *by); 
RLAPI char *TextInsert(const char *text, const char *insert, int position); 
RLAPI const char *TextJoin(const char **textList, int count, const char *delimiter); 
RLAPI const char **TextSplit(const char *text, char delimiter, int *count); 
RLAPI void TextAppend(char *text, const char *append, int *position); 
RLAPI int TextFindIndex(const char *text, const char *find); 
RLAPI const char *TextToUpper(const char *text); 
RLAPI const char *TextToLower(const char *text); 
RLAPI const char *TextToPascal(const char *text); 
RLAPI int TextToInteger(const char *text); 
RLAPI char *TextToUtf8(int *codepoints, int length); 


RLAPI int *GetCodepoints(const char *text, int *count); 
RLAPI int GetCodepointsCount(const char *text); 
RLAPI int GetNextCodepoint(const char *text, int *bytesProcessed); 
RLAPI const char *CodepointToUtf8(int codepoint, int *byteLength); 






RLAPI void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color); 
RLAPI void DrawPoint3D(Vector3 position, Color color); 
RLAPI void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color); 
RLAPI void DrawCube(Vector3 position, float width, float height, float length, Color color); 
RLAPI void DrawCubeV(Vector3 position, Vector3 size, Color color); 
RLAPI void DrawCubeWires(Vector3 position, float width, float height, float length, Color color); 
RLAPI void DrawCubeWiresV(Vector3 position, Vector3 size, Color color); 
RLAPI void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color); 
RLAPI void DrawSphere(Vector3 centerPos, float radius, Color color); 
RLAPI void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color); 
RLAPI void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color); 
RLAPI void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); 
RLAPI void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); 
RLAPI void DrawPlane(Vector3 centerPos, Vector2 size, Color color); 
RLAPI void DrawRay(Ray ray, Color color); 
RLAPI void DrawGrid(int slices, float spacing); 
RLAPI void DrawGizmo(Vector3 position); 






RLAPI Model LoadModel(const char *fileName); 
RLAPI Model LoadModelFromMesh(Mesh mesh); 
RLAPI void UnloadModel(Model model); 


RLAPI Mesh *LoadMeshes(const char *fileName, int *meshCount); 
RLAPI void ExportMesh(Mesh mesh, const char *fileName); 
RLAPI void UnloadMesh(Mesh mesh); 


RLAPI Material *LoadMaterials(const char *fileName, int *materialCount); 
RLAPI Material LoadMaterialDefault(void); 
RLAPI void UnloadMaterial(Material material); 
RLAPI void SetMaterialTexture(Material *material, int mapType, Texture2D texture); 
RLAPI void SetModelMeshMaterial(Model *model, int meshId, int materialId); 


RLAPI ModelAnimation *LoadModelAnimations(const char *fileName, int *animsCount); 
RLAPI void UpdateModelAnimation(Model model, ModelAnimation anim, int frame); 
RLAPI void UnloadModelAnimation(ModelAnimation anim); 
RLAPI bool IsModelAnimationValid(Model model, ModelAnimation anim); 


RLAPI Mesh GenMeshPoly(int sides, float radius); 
RLAPI Mesh GenMeshPlane(float width, float length, int resX, int resZ); 
RLAPI Mesh GenMeshCube(float width, float height, float length); 
RLAPI Mesh GenMeshSphere(float radius, int rings, int slices); 
RLAPI Mesh GenMeshHemiSphere(float radius, int rings, int slices); 
RLAPI Mesh GenMeshCylinder(float radius, float height, int slices); 
RLAPI Mesh GenMeshTorus(float radius, float size, int radSeg, int sides); 
RLAPI Mesh GenMeshKnot(float radius, float size, int radSeg, int sides); 
RLAPI Mesh GenMeshHeightmap(Image heightmap, Vector3 size); 
RLAPI Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize); 


RLAPI BoundingBox MeshBoundingBox(Mesh mesh); 
RLAPI void MeshTangents(Mesh *mesh); 
RLAPI void MeshBinormals(Mesh *mesh); 


RLAPI void DrawModel(Model model, Vector3 position, float scale, Color tint); 
RLAPI void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); 
RLAPI void DrawModelWires(Model model, Vector3 position, float scale, Color tint); 
RLAPI void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); 
RLAPI void DrawBoundingBox(BoundingBox box, Color color); 
RLAPI void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint); 
RLAPI void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint); 


RLAPI bool CheckCollisionSpheres(Vector3 centerA, float radiusA, Vector3 centerB, float radiusB); 
RLAPI bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2); 
RLAPI bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius); 
RLAPI bool CheckCollisionRaySphere(Ray ray, Vector3 center, float radius); 
RLAPI bool CheckCollisionRaySphereEx(Ray ray, Vector3 center, float radius, Vector3 *collisionPoint); 
RLAPI bool CheckCollisionRayBox(Ray ray, BoundingBox box); 
RLAPI RayHitInfo GetCollisionRayModel(Ray ray, Model model); 
RLAPI RayHitInfo GetCollisionRayTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3); 
RLAPI RayHitInfo GetCollisionRayGround(Ray ray, float groundHeight); 







RLAPI char *LoadText(const char *fileName); 
RLAPI Shader LoadShader(const char *vsFileName, const char *fsFileName); 
RLAPI Shader LoadShaderCode(const char *vsCode, const char *fsCode); 
RLAPI void UnloadShader(Shader shader); 

RLAPI Shader GetShaderDefault(void); 
RLAPI Texture2D GetTextureDefault(void); 
RLAPI Texture2D GetShapesTexture(void); 
RLAPI Rectangle GetShapesTextureRec(void); 
RLAPI void SetShapesTexture(Texture2D texture, Rectangle source); 


RLAPI int GetShaderLocation(Shader shader, const char *uniformName); 
RLAPI void SetShaderValue(Shader shader, int uniformLoc, const void *value, int uniformType); 
RLAPI void SetShaderValueV(Shader shader, int uniformLoc, const void *value, int uniformType, int count); 
RLAPI void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat); 
RLAPI void SetShaderValueTexture(Shader shader, int uniformLoc, Texture2D texture); 
RLAPI void SetMatrixProjection(Matrix proj); 
RLAPI void SetMatrixModelview(Matrix view); 
RLAPI Matrix GetMatrixModelview(void); 
RLAPI Matrix GetMatrixProjection(void); 



RLAPI Texture2D GenTextureCubemap(Shader shader, Texture2D map, int size); 
RLAPI Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size); 
RLAPI Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size); 
RLAPI Texture2D GenTextureBRDF(Shader shader, int size); 


RLAPI void BeginShaderMode(Shader shader); 
RLAPI void EndShaderMode(void); 
RLAPI void BeginBlendMode(int mode); 
RLAPI void EndBlendMode(void); 


RLAPI void InitVrSimulator(void); 
RLAPI void CloseVrSimulator(void); 
RLAPI void UpdateVrTracking(Camera *camera); 
RLAPI void SetVrConfiguration(VrDeviceInfo info, Shader distortion); 
RLAPI bool IsVrSimulatorReady(void); 
RLAPI void ToggleVrMode(void); 
RLAPI void BeginVrDrawing(void); 
RLAPI void EndVrDrawing(void); 






RLAPI void InitAudioDevice(void); 
RLAPI void CloseAudioDevice(void); 
RLAPI bool IsAudioDeviceReady(void); 
RLAPI void SetMasterVolume(float volume); 


RLAPI Wave LoadWave(const char *fileName); 
RLAPI Sound LoadSound(const char *fileName); 
RLAPI Sound LoadSoundFromWave(Wave wave); 
RLAPI void UpdateSound(Sound sound, const void *data, int samplesCount);
RLAPI void UnloadWave(Wave wave); 
RLAPI void UnloadSound(Sound sound); 
RLAPI void ExportWave(Wave wave, const char *fileName); 
RLAPI void ExportWaveAsCode(Wave wave, const char *fileName); 


RLAPI void PlaySound(Sound sound); 
RLAPI void StopSound(Sound sound); 
RLAPI void PauseSound(Sound sound); 
RLAPI void ResumeSound(Sound sound); 
RLAPI void PlaySoundMulti(Sound sound); 
RLAPI void StopSoundMulti(void); 
RLAPI int GetSoundsPlaying(void); 
RLAPI bool IsSoundPlaying(Sound sound); 
RLAPI void SetSoundVolume(Sound sound, float volume); 
RLAPI void SetSoundPitch(Sound sound, float pitch); 
RLAPI void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels); 
RLAPI Wave WaveCopy(Wave wave); 
RLAPI void WaveCrop(Wave *wave, int initSample, int finalSample); 
RLAPI float *GetWaveData(Wave wave); 


RLAPI Music LoadMusicStream(const char *fileName); 
RLAPI void UnloadMusicStream(Music music); 
RLAPI void PlayMusicStream(Music music); 
RLAPI void UpdateMusicStream(Music music); 
RLAPI void StopMusicStream(Music music); 
RLAPI void PauseMusicStream(Music music); 
RLAPI void ResumeMusicStream(Music music); 
RLAPI bool IsMusicPlaying(Music music); 
RLAPI void SetMusicVolume(Music music, float volume); 
RLAPI void SetMusicPitch(Music music, float pitch); 
RLAPI void SetMusicLoopCount(Music music, int count); 
RLAPI float GetMusicTimeLength(Music music); 
RLAPI float GetMusicTimePlayed(Music music); 


RLAPI AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); 
RLAPI void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount); 
RLAPI void CloseAudioStream(AudioStream stream); 
RLAPI bool IsAudioStreamProcessed(AudioStream stream); 
RLAPI void PlayAudioStream(AudioStream stream); 
RLAPI void PauseAudioStream(AudioStream stream); 
RLAPI void ResumeAudioStream(AudioStream stream); 
RLAPI bool IsAudioStreamPlaying(AudioStream stream); 
RLAPI void StopAudioStream(AudioStream stream); 
RLAPI void SetAudioStreamVolume(AudioStream stream, float volume); 
RLAPI void SetAudioStreamPitch(AudioStream stream, float pitch); 
RLAPI void SetAudioStreamBufferSizeDefault(int size); 
