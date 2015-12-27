
//*****MODEL LOADING*****//
const char* modelPath = "../data/Creeper/creeper.obj";
std::vector<Vector4> vertices;
std::vector<std::array<float, 2>> uvs;
std::vector<Vector4> normales;
std::vector<ModelFace> faces;

{
  std::fstream file;
  file.open(modelPath, std::ios_base::in);
  assert(file.is_open());
  std::string input;

  while (std::getline(file, input))
  {
    std::stringstream stream;
    stream.str(input);
    char lineHeader;
    stream >> lineHeader;
    switch (lineHeader)
    {
      case 'v':
      {
        char secondChar;
        stream.get(secondChar);
        switch (secondChar)
        {
          case ' ':
          {
            float x, y, z;
            stream >> x >> y >> z;

            vertices.push_back(Vector4 { x, y, z, 1.0f });
          } break;
          case 't':
          {
            float u, v;
            stream >> u >> v;

            uvs.push_back({{ u, v }});
          } break;
          case 'n':
          {
            float x, y, z;
            stream >> x >> y >> z;

            normales.push_back(Vector4 { x, y, z, 0 });
          } break;
        }
      } break;
      case 'f':
      {
        u32 v1, v2, v3;
        u32 uv1, uv2, uv3;
        u32 n1, n2, n3;

        stream >> v1;
        stream.ignore(u32(-1), '/');
        stream >> uv1;
        stream.ignore(u32(-1), '/');
        stream >> n1;

        stream >> v2;
        stream.ignore(u32(-1), '/');
        stream >> uv2;
        stream.ignore(u32(-1), '/');
        stream >> n2;

        stream >> v3;
        stream.ignore(u32(-1), '/');
        stream >> uv3;
        stream.ignore(u32(-1), '/');
        stream >> n3;

        ModelFace face
        {
          {  v1 - 1,  v2 - 1,  v3 - 1 },
          { uv1 - 1, uv2 - 1, uv3 - 1 },
          {  n1 - 1,  n2 - 1,  n3 - 1 }
        };
        faces.push_back(face);

        stream.ignore(u32(-1), ' ');
        u32 v4;
        if (stream >> v4)
        {
          // I don't want to waste time on quads now
          assert(false);
          //faces.push_back(ModelFace { { v3 - 1, v4 - 1, v1 - 1 } });
        }
      } break;
    }
  }
}

//*****LOADING TEXTURE*****//
Color32* colorTexture = nullptr;
u32 colorTextureWidth = 0;
u32 colorTextureHeight = 0;
const char* texturePath = "../data/Creeper/color.bmp";
{
  std::fstream texture;
  texture.open(texturePath, std::ios_base::in | std::ios_base::binary);
  assert(texture.is_open());

  BITMAPFILEHEADER textureHeader;
  const u16 bitmapFileType = (u16('M') << 8) | u16('B');
  texture.read((char*)&textureHeader, sizeof(textureHeader));
  assert(textureHeader.bfType == bitmapFileType);

  BITMAPINFOHEADER textureInfo;
  texture.read((char*)&textureInfo, sizeof(textureInfo));
  assert(textureInfo.biBitCount == 24);
  assert(textureInfo.biCompression == BI_RGB);
  colorTextureWidth = textureInfo.biWidth;
  colorTextureHeight = textureInfo.biHeight;
  u32 texturePixelsCount = colorTextureWidth * colorTextureHeight;
  colorTexture = new Color32[texturePixelsCount];

  /* texture.seekg(textureHeader.bfOffBits); */
  texture.ignore(textureHeader.bfOffBits - sizeof(textureHeader) - sizeof(textureInfo));
  u32 paddingBytes = 4 - ((3 * colorTextureWidth) % 4);
  paddingBytes %= 4;
  for (u32 y = 0; y < colorTextureHeight; ++y)
  {
    for (u32 x = 0; x < colorTextureWidth; ++x)
    {
      u8 r, g, b;
      // fuck you, C++ and STL...
      texture.read((char*)&b, 1);
      texture.read((char*)&g, 1);
      texture.read((char*)&r, 1);
      colorTexture[y * colorTextureWidth + x] = { r, g, b, 0 };
    }

    if (paddingBytes)
      texture.ignore(paddingBytes);
  }
}

