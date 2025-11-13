#include "IEHelper.h"
#include "Tool_Defines.h"

static bool ieq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    return true;
}
static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
static std::string StemLower(const std::filesystem::path& p) {
    return ToLower(p.stem().string());
}
static bool HasImgExtCI(const std::filesystem::path& p) {
    static const char* exts[] = { ".png",".jpg",".jpeg",".tga",".bmp",".dds",".tif",".tiff" };
    std::string e = ToLower(p.extension().string());
    for (auto* x : exts) if (e == x) return true;
    return false;
}
static inline bool IsImgExt(const std::string& e) {
    return _stricmp(e.c_str(), ".png") == 0 || _stricmp(e.c_str(), ".jpg") == 0 || _stricmp(e.c_str(), ".jpeg") == 0 ||
        _stricmp(e.c_str(), ".tga") == 0 || _stricmp(e.c_str(), ".bmp") == 0 || _stricmp(e.c_str(), ".dds") == 0 ||
        _stricmp(e.c_str(), ".tif") == 0 || _stricmp(e.c_str(), ".tiff") == 0;
}
static std::string FindByStemCaseInsensitive(const std::filesystem::path& dir, const std::string& stemWanted) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) return {};
    for (auto& e : std::filesystem::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        const auto& p = e.path();
        if (!IsImgExt(p.extension().string())) continue;
        if (ieq(p.stem().string(), stemWanted)) return p.string();
    }
    return {};
}

static _float4x4 IdentityF4x4() { _float4x4 r{}; r._11 = r._22 = r._33 = r._44 = 1.0f; return r; }

static uint32_t Fnv1a32(const std::string& s) {
    uint32_t h = 0x811C9DC5u;
    for (unsigned char c : s) { h ^= c; h *= 0x01000193u; }
    return h;
}
static bool IsDir(const std::string& p) {
    std::error_code ec; return std::filesystem::is_directory(p, ec);
}
static inline void NormalizeSlashes(std::string& s) { for (char& c : s) if (c == '\\') c = '/'; }

static inline bool ExistsFile(const std::filesystem::path& p) {
    std::error_code ec; return std::filesystem::exists(p, ec) && std::filesystem::is_regular_file(p, ec);
}
static std::filesystem::path ResolveRelative(std::string p, const std::filesystem::path& fbxFolder) {
    if (p.rfind("//", 0) == 0) p = p.substr(2);
    NormalizeSlashes(p);
    std::filesystem::path ph = p;
    if (!ph.is_absolute()) ph = fbxFolder / ph;
    return ph.lexically_normal();
}
static std::string ToU8(const aiString& s) { return std::string(s.C_Str()); }
static std::string PickFromTextures(const std::filesystem::path& texDir, TextureType hint) {
    if (!std::filesystem::exists(texDir)) return {};
    auto score = [&](const std::filesystem::path& f)->int {
        std::string n = f.filename().string(); std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        auto has = [&](std::initializer_list<const char*> ks) { for (auto k : ks) if (n.find(k) != std::string::npos) return true; return false; };
        if (hint == TextureType::Diffuse && (has({ "albedo","basecolor","diffuse","color","col","base_color" }) || has({ "tool","tools","nolight","unlit" }))) return 3;
        if (hint == TextureType::Normal && has({ "normal","nrm","norm","_n","-n" })) return 3;
        if (hint == TextureType::Specular && has({ "spec","specular","metal","rough","gloss","mrao","orm","ao" })) return 2;
        if (hint == TextureType::Emissive && has({ "emis","emissive","glow" })) return 2;
        return 1;
        };
    std::vector<std::filesystem::path> cands;
    for (auto& e : std::filesystem::directory_iterator(texDir))
        if (e.is_regular_file() && IsImgExt(e.path().extension().string()))
            cands.push_back(e.path());
    if (cands.empty()) return {};
    std::sort(cands.begin(), cands.end(), [&](auto& a, auto& b) {return score(a) > score(b); });
    return cands.front().string();
}

static std::string PickFromFolderByHint(const std::filesystem::path& dir, TextureType hint) {
    // 우선순위 키워드
    auto has = [](std::string s, std::initializer_list<const char*> keys) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        for (auto k : keys) if (s.find(k) != std::string::npos) return true;
        return false;
        };
    static const char* exts[] = { ".png",".jpg",".jpeg",".tga",".bmp",".dds",".tif",".tiff" };

    // 1) 후보 수집
    std::vector<std::filesystem::path> imgs;
    for (auto& e : std::filesystem::directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        auto ext = e.path().extension().string();
        for (auto x : exts) if (_stricmp(ext.c_str(), x) == 0) { imgs.push_back(e.path()); break; }
    }
    if (imgs.empty()) return {};

    // 2) 힌트로 필터링
    auto score = [&](const std::filesystem::path& p)->int {
        std::string n = p.filename().string();
        switch (hint) {
        case TextureType::Diffuse:
            if (has(n, { "albedo","basecolor","diffuse","color","col","base_color" })) return 3;
            break;
        case TextureType::Normal:
            if (has(n, { "normal","nrm","norm","_n","-n" })) return 3;
            break;
        case TextureType::Specular:
            if (has(n, { "spec","specular","metal","rough","gloss","mrao" })) return 2;
            break;
        case TextureType::Emissive:
            if (has(n, { "emis","emissive","glow" })) return 2;
            break;
        default: break;
        }
        return 1; // 기타
        };
    std::sort(imgs.begin(), imgs.end(), [&](auto& a, auto& b) { return score(a) > score(b); });
    return imgs.front().string();
}
static std::string CollapseRepeatedDotTokens(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    std::string cur, prev;
    for (size_t i = 0; i <= in.size(); ++i) {
        char ch = (i < in.size() ? in[i] : '.'); // 마지막 토큰 플러시
        if (ch == '.') {
            if (!cur.empty()) {
                if (cur != prev) {
                    if (!out.empty()) out.push_back('.');
                    out += cur;
                    prev = cur;
                }
                else {
                    // 동일 토큰 반복인 경우: skip
                }
                cur.clear();
            }
            else {
                // 연속 '.' 는 하나로
                if (!out.empty() && out.back() != '.') out.push_back('.');
            }
        }
        else {
            cur.push_back(ch);
        }
    }
    // 끝이 '.' 로 끝나면 제거
    while (!out.empty() && out.back() == '.') out.pop_back();
    return out.empty() ? in : out;
}
static void SanitizeBadChars(std::string& s) { for (char& c : s) { switch (c) { case ':':case '*':case '?':case '\"':case '<':case '>':case '|': c = '_'; break; } } }
static std::string SanitizeName(const std::string& name, size_t maxLen = 128) {
    std::string s = CollapseRepeatedDotTokens(name); SanitizeBadChars(s);
    while (!s.empty() && (s.back() == ' ' || s.back() == '.')) s.pop_back();
    size_t st = 0; while (st < s.size() && s[st] == ' ') ++st; if (st) s.erase(0, st);
    if (s.size() <= maxLen) return s;
    // truncate with ~hash (간단화)
    uint32_t h = 0x811C9DC5u; for (unsigned char c : s) { h ^= c; h *= 0x01000193u; }
    char tag[10]; snprintf(tag, sizeof(tag), "~%08X", h);
    size_t keep = maxLen > 9 ? maxLen - 9 : 0; if (!keep) return std::string(tag);
    std::string cut = s.substr(0, keep); while (!cut.empty() && cut.back() == '.') cut.pop_back();
    return cut + tag;
}
struct AnimPruneThreshold {
    float posEps = 1e-4f;          // 한 채널의 위치 변경 허용치 (단위: 원본 단위)
    float rotDegEps = 0.1f;        // 한 채널의 회전 변경 허용치 (deg)
    float scaleEps = 1e-4f;        // 한 채널의 스케일 변경 허용치
    bool  requireRootMotion = false; // true면 root bone 이동/회전 없으면 제거
    std::string rootName = "root";   // root 본 이름 추정
};
static std::unordered_set<std::string> CollectAnimBoneNames(const ModelData& model) {
    std::unordered_set<std::string> s;
    for (auto& a : model.animations)
        for (auto& ch : a.channels)
            s.insert(ch.nodeName);
    return s;
}
static void BuildParentMapRec(const NodeData& n, const std::string& parent, std::unordered_map<std::string, std::string>& out) {
    out[n.name] = parent;
    for (auto& c : n.children)
        BuildParentMapRec(c, n.name, out);
}
static std::unordered_map<std::string, std::string> BuildParentMapFromModel(const ModelData& model) {
    std::unordered_map<std::string, std::string> m;
    BuildParentMapRec(model.rootNode, std::string{}, m);
    return m;
}
static std::string FindUsableAncestor(const std::string& bone,
    const std::unordered_set<std::string>& animBones,
    const std::unordered_map<std::string, std::string>& parentMap,
    const std::string& rootKeep = {}) {
    std::string cur = bone;
    // 바로 부모부터 검사
    auto it = parentMap.find(cur);
    while (it != parentMap.end()) {
        const std::string& p = it->second; // parent name (빈 문자열이면 루트 상실)
        if (p.empty()) {
            // 루트 이름을 강제로 보존하고 싶으면 허용
            return (rootKeep.empty() ? std::string{} : rootKeep);
        }
        if (animBones.count(p)) return p;
        it = parentMap.find(p);
    }
    return std::string{};
}
static float AngleBetweenQuatDeg(const _float4& a, const _float4& b) {
    // 두 쿼터니언 사이 각도 (deg)
    // q, -q 동일 취급
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    dot = std::clamp(dot, -1.0f, 1.0f);
    float ang = 2.0f * acosf(fabsf(dot)); // rad
    return XMConvertToDegrees(ang);
}
static void RemapAndPruneMeshToAnimSet(
    MeshData& mesh,
    const std::unordered_set<std::string>& animBones,
    const std::unordered_map<std::string, std::string>& parentMap,
    const std::string& rootKeepName, // 예: "root" 비워도 됨
    bool strictParentOne,            // true: 부모 1.0 고정, false: 부모로 가중치 이동 후 정규화
    unsigned maxInfluences = 4)
{
    const uint32_t numV = (uint32_t)mesh.positions.size();
    // 원본: boneIndex -> weights
    std::unordered_map<std::string, uint32_t> boneIndexByName;
    boneIndexByName.reserve(mesh.bones.size());
    for (uint32_t i = 0; i < mesh.bones.size(); ++i)
        boneIndexByName[mesh.bones[i].name] = i;

    // [vtx 중심] (boneIndex, weight) 리스트
    std::vector<std::vector<std::pair<uint32_t, float>>> vtx(numV);
    for (uint32_t b = 0; b < mesh.bones.size(); ++b) {
        for (auto& w : mesh.bones[b].weights) {
            if (w.vertexId < numV && w.weight > 0.f)
                vtx[w.vertexId].emplace_back(b, w.weight);
        }
    }

    // 처리
    for (uint32_t v = 0; v < numV; ++v) {
        auto& inf = vtx[v];
        if (inf.empty()) continue;

        // 누적 버퍼(승격/합치기)
        std::unordered_map<uint32_t, float> acc;
        acc.reserve(inf.size() + 2);

        if (strictParentOne) {
            // 이 버텍스에서 "애니에 없는 본" 중 첫 후보의 usable 조상 결정
            uint32_t targetB = UINT32_MAX;
            // 후보 이름
            std::string targetName;
            for (auto& [b, w] : inf) {
                const std::string& bn = mesh.bones[b].name;
                if (animBones.count(bn) == 0) {
                    targetName = FindUsableAncestor(bn, animBones, parentMap, rootKeepName);
                    break;
                }
            }
            if (!targetName.empty()) {
                // 본 인덱스 찾기 (없으면 나중 재배치)
                auto it = boneIndexByName.find(targetName);
                if (it != boneIndexByName.end()) targetB = it->second;
            }
            // 조상도 못 찾으면: 남아있는 애니 본 중 최대 가중치 본 선택
            if (targetB == UINT32_MAX) {
                float bestW = -1.f; uint32_t bestB = 0; bool found = false;
                for (auto& [b, w] : inf) {
                    if (animBones.count(mesh.bones[b].name)) {
                        if (w > bestW) { bestW = w; bestB = b; found = true; }
                    }
                }
                targetB = found ? bestB : inf.front().first; // 그래도 없으면 아무거나
            }
            acc[targetB] = 1.0f; // 부모 1.0 고정
        }
        else {
            // 없는 본 가중치를 usable 조상으로 ADD
            for (auto& [b, w] : inf) {
                const std::string& bn = mesh.bones[b].name;
                if (animBones.count(bn)) {
                    acc[b] += w;
                }
                else {
                    std::string anc = FindUsableAncestor(bn, animBones, parentMap, rootKeepName);
                    auto it = (!anc.empty()) ? boneIndexByName.find(anc) : boneIndexByName.end();
                    if (it != boneIndexByName.end()) acc[it->second] += w;
                    // 못 찾으면 버림(0)
                }
            }
            // 정규화 + 영향 제한
            std::vector<std::pair<uint32_t, float>> tmp(acc.begin(), acc.end());
            std::sort(tmp.begin(), tmp.end(), [](auto& a, auto& b) { return a.second > b.second; });
            if (tmp.size() > maxInfluences) tmp.resize(maxInfluences);
            float s = 0.f; for (auto& p : tmp) s += p.second;
            inf.swap(tmp);
            if (s > 0.f) for (auto& p : inf) p.second /= s;
            continue;
        }

        // strictParentOne 모드일 때 inf 갱신
        inf.clear();
        for (auto& kv : acc) inf.emplace_back(kv.first, kv.second);
    }

    // 본별 리스트 재구성(이때 '애니에 있는 본'만 남김)
    // 1) 기존 본의 weights 비우기
    for (auto& b : mesh.bones) b.weights.clear();

    // 2) vtx -> bones로 되돌리되, animBones에 없는 본은 스킵
    for (uint32_t v = 0; v < numV; ++v) {
        for (auto& [b, w] : vtx[v]) {
            if (w <= 0.f) continue;
            if (animBones.count(mesh.bones[b].name) == 0) continue; // PRUNE
            mesh.bones[b].weights.push_back(VertexWeight{ v, w });
        }
    }

    // 3) 가중치가 하나도 없는 본 제거 + 인덱스 재구성
    std::vector<MeshBone> kept;
    kept.reserve(mesh.bones.size());
    for (auto& b : mesh.bones) {
        if (!b.weights.empty() && animBones.count(b.name)) {
            kept.push_back(std::move(b));
        }
    }
    mesh.bones.swap(kept);
}
static void PruneModelBonesToAnimSet(
    ModelData& model,
    const std::string& rootKeepName, // 예: "root" (없으면 "")
    bool strictParentOne,            // true면 부모 1.0 고정
    unsigned maxInfluences = 4)
{
    auto animBones = CollectAnimBoneNames(model);
    auto parentMap = BuildParentMapFromModel(model);

    for (auto& mesh : model.meshes) {
        RemapAndPruneMeshToAnimSet(mesh, animBones, parentMap, rootKeepName, strictParentOne, maxInfluences);
    }

    // Global Bones(outModel.bones)도 '사용 중인 본'만 남기기
    // (메쉬에 실제 남아있는 본 이름들의 합집합)
    std::unordered_set<std::string> used;
    for (auto& mesh : model.meshes)
        for (auto& b : mesh.bones) used.insert(b.name);

    // 기존 offsetMatrix를 잃지 않도록 name->offset 보관
    std::unordered_map<std::string, _float4x4> offsetByName;
    for (auto& b : model.bones) offsetByName[b.name] = b.offsetMatrix;

    std::vector<BoneData> pruned;
    pruned.reserve(used.size());
    for (auto& name : used) {
        BoneData bd;
        bd.name = name;
        auto it = offsetByName.find(name);
        bd.offsetMatrix = (it != offsetByName.end()) ? it->second : IdentityF4x4();
        pruned.push_back(std::move(bd));
    }
    // 정렬(옵션): 이름 순으로 깔끔하게
    std::sort(pruned.begin(), pruned.end(), [](const BoneData& a, const BoneData& b) {
        return a.name < b.name;
        });
    model.bones.swap(pruned);
}
static bool IsStaticChannel(const ChannelData& ch, const AnimPruneThreshold& th) {
    // 위치
    float maxPosDelta = 0.f;
    for (size_t i = 1; i < ch.positionKeys.size(); ++i) {
        auto& p0 = ch.positionKeys[i - 1].value;
        auto& p1 = ch.positionKeys[i].value;
        float dx = p1.x - p0.x, dy = p1.y - p0.y, dz = p1.z - p0.z;
        maxPosDelta = max(maxPosDelta, fabsf(dx));
        maxPosDelta = max(maxPosDelta, fabsf(dy));
        maxPosDelta = max(maxPosDelta, fabsf(dz));
        if (maxPosDelta > th.posEps) return false;
    }
    // 회전
    float maxRotDeg = 0.f;
    for (size_t i = 1; i < ch.rotationKeys.size(); ++i) {
        maxRotDeg = max(maxRotDeg, AngleBetweenQuatDeg(ch.rotationKeys[i - 1].value,
            ch.rotationKeys[i].value));
        if (maxRotDeg > th.rotDegEps) return false;
    }
    // 스케일
    float maxScaleDelta = 0.f;
    for (size_t i = 1; i < ch.scalingKeys.size(); ++i) {
        auto& s0 = ch.scalingKeys[i - 1].value;
        auto& s1 = ch.scalingKeys[i].value;
        float dx = s1.x - s0.x, dy = s1.y - s0.y, dz = s1.z - s0.z;
        maxScaleDelta = max(maxScaleDelta, fabsf(dx));
        maxScaleDelta = max(maxScaleDelta, fabsf(dy));
        maxScaleDelta = max(maxScaleDelta, fabsf(dz));
        if (maxScaleDelta > th.scaleEps) return false;
    }
    // 여기까지 오면 이 채널은 정지로 간주
    return true;
}

static bool AnimationIsStatic(const AnimationData& anim,
    const AnimPruneThreshold& th,
    bool* outOnlyOneKey = nullptr)
{
    // 1) 전체적으로 키가 사실상 1개뿐(=포즈)인지 확인
    size_t totalKeys = 0;
    for (auto& ch : anim.channels) {
        totalKeys += ch.positionKeys.size();
        totalKeys += ch.rotationKeys.size();
        totalKeys += ch.scalingKeys.size();
    }
    if (outOnlyOneKey) *outOnlyOneKey = (totalKeys <= 1);
    if (totalKeys <= 1) return true;

    // 2) 루트 모션 강제 조건
    if (th.requireRootMotion) {
        for (auto& ch : anim.channels) {
            if (ch.nodeName == th.rootName) {
                if (!IsStaticChannel(ch, th)) {
                    // 루트에서 변화가 있다면 통과
                    goto CHECK_ALL_CHANNELS;
                }
                else {
                    // 루트에 변화가 없으면 정지로 간주
                    return true;
                }
            }
        }
        // 루트 채널이 없으면 보수적으로 정지로 간주하지 않음
    }

CHECK_ALL_CHANNELS:
    // 3) 모든 채널이 정지면 이 애니는 정지
    bool anyDynamic = false;
    for (auto& ch : anim.channels) {
        if (!IsStaticChannel(ch, th)) { anyDynamic = true; break; }
    }
    return !anyDynamic;
}

static void PruneStaticAnimations(ModelData& model,
    const AnimPruneThreshold& th,
    const std::vector<std::string>& keepNameContains = {})
{
    std::vector<AnimationData> filtered;
    filtered.reserve(model.animations.size());

    for (auto& a : model.animations) {
        // 화이트리스트(이름 일부 포함)는 무조건 유지
        bool keepByName = false;
        for (auto& key : keepNameContains) {
            if (!key.empty() && a.name.find(key) != std::string::npos) {
                keepByName = true; break;
            }
        }
        if (keepByName) {
            filtered.push_back(a);
            continue;
        }

        bool onlyOne = false;
        bool isStatic = AnimationIsStatic(a, th, &onlyOne);
        if (isStatic) {
            char buf[256];
            sprintf_s(buf, "[Prune] drop anim '%s' (static%s)\n",
                a.name.c_str(), onlyOne ? "/oneKey" : "");
            OutputDebugStringA(buf);
            continue;
        }
        filtered.push_back(a);
    }

    model.animations.swap(filtered);
}
// ★ 안전 행렬/쿼터니언 변환 유틸
static _float4x4 ToF4x4(const aiMatrix4x4& m) {
    _float4x4 r;
    r._11 = m.a1; r._12 = m.a2; r._13 = m.a3; r._14 = m.a4;
    r._21 = m.b1; r._22 = m.b2; r._23 = m.b3; r._24 = m.b4;
    r._31 = m.c1; r._32 = m.c2; r._33 = m.c3; r._34 = m.c4;
    r._41 = m.d1; r._42 = m.d2; r._43 = m.d3; r._44 = m.d4;
    return r;
}

static void NormalizeQuat(_float4& q) { float l = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w); if (l > 1e-8f) { q.x /= l; q.y /= l; q.z /= l; q.w /= l; } }


// ★ 임베디드 텍스처/경로 보정
static std::string NormalizePath(const std::string& p);
static std::string JoinPath(const std::filesystem::path& base, const std::string& rel) {
    return (base / rel).lexically_normal().string();
}

static std::string DumpEmbeddedTexture(const aiTexture* tex, const std::filesystem::path& outDir, int index) {
    if (!tex) return {};
    std::filesystem::create_directories(outDir);

    // 압축 이미지(mHeight==0) → 원본 포맷 그대로 저장
    if (tex->mHeight == 0) {
        // 포맷 힌트(tex->achFormatHint)로 확장자 유추
        std::string ext = ".bin";
        if (tex->achFormatHint[0]) {
            ext = "." + std::string(tex->achFormatHint);
        }
        std::string outPath = (outDir / ("embedded_" + std::to_string(index) + ext)).string();
        std::ofstream ofs(outPath, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth); // mWidth=byte size
        return outPath;
    }
    else {
        // 비압축 RGBA 텍스처(이미지 포맷이 아님). 필요하면 DDS/PNG 인코딩 추가.
        std::string outPath = (outDir / ("embedded_" + std::to_string(index) + ".rawrgba")).string();
        std::ofstream ofs(outPath, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth * tex->mHeight * 4);
        return outPath;
    }
}
static bool FileExists(const std::string& p) {
    std::error_code ec; return std::filesystem::exists(p, ec);
}
static std::string FixTexturePath_FBX(std::string raw, const std::filesystem::path& fbxFilePath, TextureType hint)
{
    if (raw.empty()) return {};
    const auto fbxFolder = fbxFilePath.parent_path();

    // 1) .fbm 토큰/폴더명 → fbxFolder/textures 로 리다이렉트
    {
        std::filesystem::path ph = raw;
        const std::string fbmName = fbxFilePath.stem().string() + ".fbm";
        const std::string last = ph.filename().string();
        if (_stricmp(ph.extension().string().c_str(), ".fbm") == 0 || _stricmp(last.c_str(), fbmName.c_str()) == 0) {
            auto tex = PickFromTextures(fbxFolder / "textures", hint);
            return tex;
        }
    }

    // 2) 상대경로 정규화
    auto ph = ResolveRelative(raw, fbxFolder);

    // 2-1) 확장자 있고 파일 존재 → OK
    if (!ph.extension().empty() && ExistsFile(ph))
        return ph.string();

    // 2-2) 확장자 없으면 같은 폴더 및 textures/에서 스템(case-insensitive) 매칭
    if (ph.extension().empty()) {
        const std::string stem = ph.stem().string();
        if (auto s = FindByStemCaseInsensitive(ph.parent_path(), stem); !s.empty()) return s;
        if (auto s = FindByStemCaseInsensitive(fbxFolder / "textures", stem); !s.empty()) return s;
        // 그래도 못 찾으면 힌트 기반 폴백
        return PickFromTextures(fbxFolder / "textures", hint);
    }

    // 2-3) 확장자는 있는데 파일이 없으면 textures/ 폴백
    if (!ExistsFile(ph))
        return PickFromTextures(fbxFolder / "textures", hint);

    return ph.string();
}
static std::string FindByStemCaseInsensitiveRecursive(
    const std::filesystem::path& texRoot,
    const std::string& materialName)
{
    if (!std::filesystem::exists(texRoot)) return {};
    const std::string target = ToLower(materialName);
    for (auto it = std::filesystem::recursive_directory_iterator(texRoot);
        it != std::filesystem::recursive_directory_iterator(); ++it)
    {
        if (!it->is_regular_file()) continue;
        const auto& p = it->path();
        if (!HasImgExtCI(p)) continue;
        if (StemLower(p) == target) return p.lexically_normal().string();
    }
    return {};
}
static int ScoreTextureName(const std::string& nameLower,
    const std::string& matLower,
    TextureType tt)
{
    int score = 0;
    if (nameLower.find(matLower) != std::string::npos) score += 10;

    auto has = [&](std::initializer_list<const char*> keys) {
        for (auto k : keys) if (nameLower.find(k) != std::string::npos) return true;
        return false;
        };
    switch (tt) {
    case TextureType::Diffuse:
        if (has({ "albedo","basecolor","diffuse","color","col","base_color" })) score += 5;
        break;
    case TextureType::Normal:
        if (has({ "normal","nrm","norm","_n","-n" })) score += 5;
        break;
    case TextureType::Specular:
        if (has({ "spec","specular","metal","rough","gloss","orm","mrao" })) score += 4;
        break;
    case TextureType::Emissive:
        if (has({ "emis","emissive","glow" })) score += 4;
        break;
    default: break;
    }

    // 확장자 선호 (png > jpg/jpeg > tga > dds > others)
    if (nameLower.ends_with(".png")) score += 3;
    else if (nameLower.ends_with(".jpg") || nameLower.ends_with(".jpeg")) score += 2;
    else if (nameLower.ends_with(".tga")) score += 1;

    return score;
}
static std::string FuzzyFindTexture(const std::filesystem::path& dir, const std::string& name, TextureType hint) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) return {};
    auto norm = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        s.erase(std::remove_if(s.begin(), s.end(), [](char c) { return c == ' ' || c == '-' || c == '_'; }), s.end());
        return s;
        };
    const std::string key = norm(name);

    auto score = [&](const std::filesystem::path& p)->int {
        int sc = 0;
        std::string stem = p.stem().string();
        std::string ns = norm(stem);
        if (ns.find(key) != std::string::npos) sc += 10;          // 재질명 포함
        // 힌트 키워드 가산점
        std::string low = stem; std::transform(low.begin(), low.end(), low.begin(), ::tolower);
        if (hint == TextureType::Diffuse) {
            if (low.find("albedo") != std::string::npos || low.find("diffuse") != std::string::npos || low.find("basecolor") != std::string::npos || low.find("color") != std::string::npos) sc += 3;
        }
        else if (hint == TextureType::Normal) {
            if (low.find("normal") != std::string::npos || low.find("_n") != std::string::npos || low.find("nrm") != std::string::npos) sc += 3;
        }
        else if (hint == TextureType::Specular) {
            if (low.find("spec") != std::string::npos || low.find("rough") != std::string::npos || low.find("metal") != std::string::npos || low.find("orm") != std::string::npos) sc += 2;
        }
        else if (hint == TextureType::Emissive) {
            if (low.find("emis") != std::string::npos || low.find("glow") != std::string::npos) sc += 2;
        }
        return sc;
        };

    std::filesystem::path best; int bestSc = -1;
    for (auto& e : std::filesystem::recursive_directory_iterator(dir)) {
        if (!e.is_regular_file()) continue;
        const auto& p = e.path();
        if (!IsImgExt(p.extension().string())) continue;
        int sc = score(p);
        if (sc > bestSc) { bestSc = sc; best = p; }
    }
    return (bestSc >= 0) ? best.string() : std::string{};
}
bool IEHelper::ImportFBX(const std::string& filePath, ModelData& outModel)
{
    Assimp::Importer importer;
    importer.FreeScene();
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

    const unsigned flags = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality;
    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || !scene->mRootNode) return false;

    // === 초기화 ===
    outModel = {};
    outModel.modelDataFilePath = filePath;

    // === 헬퍼 ===
    auto SanitizeLower = [&](std::string s)->std::string {
        s = SanitizeName(s.c_str());
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        return s;
        };

    auto TryLoadAllTextureTypes = [&](const aiMaterial* mat, MaterialData& dst)
        {
            struct Map { aiTextureType src; TextureType dst; };
            const Map table[] = {
                { aiTextureType_BASE_COLOR,        TextureType::Diffuse },
                { aiTextureType_DIFFUSE,           TextureType::Diffuse },
                { aiTextureType_NORMAL_CAMERA,     TextureType::Normal },
                { aiTextureType_HEIGHT,            TextureType::Normal },
                { aiTextureType_METALNESS,         TextureType::Specular },      // 현 구조 유지
                { aiTextureType_DIFFUSE_ROUGHNESS, TextureType::Specular },      // (필요시 PBR 분리로 갈 수 있음)
                { aiTextureType_LIGHTMAP,          TextureType::Specular },
                { aiTextureType_SPECULAR,          TextureType::Specular },
                { aiTextureType_EMISSIVE,          TextureType::Emissive },
                { aiTextureType_EMISSION_COLOR,    TextureType::Emissive },
            };

            auto addTex = [&](aiTextureType tt, TextureType et)
                {
                    const unsigned cnt = mat->GetTextureCount(tt);
                    for (unsigned t = 0; t < cnt; ++t) {
                        aiString s; if (mat->GetTexture(tt, t, &s) != AI_SUCCESS) continue;
                        std::string fixed = FixTexturePath_FBX(s.C_Str(), std::filesystem::path(filePath), et);
                        if (!fixed.empty()) dst.texturePaths[et].push_back(fixed);
                    }
                };
            for (auto& m : table) addTex(m.src, m.dst);

            // 아무것도 못 찾았으면 파일명 추론(최소보조)
            bool any = false; for (int et = 0; et < (int)TextureType::End; ++et) any |= !dst.texturePaths[et].empty();
            if (!any) {
                for (int tt = (int)aiTextureType_NONE + 1; tt <= (int)aiTextureType_UNKNOWN; ++tt) {
                    const unsigned cnt = mat->GetTextureCount((aiTextureType)tt);
                    for (unsigned t = 0; t < cnt; ++t) {
                        aiString s; if (mat->GetTexture((aiTextureType)tt, t, &s) != AI_SUCCESS) continue;
                        std::string path = s.C_Str(), lower = path;
                        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                        TextureType guess = TextureType::Diffuse;
                        if (lower.find("normal") != std::string::npos || lower.find("_n") != std::string::npos) guess = TextureType::Normal;
                        else if (lower.find("rough") != std::string::npos || lower.find("metal") != std::string::npos || lower.find("spec") != std::string::npos || lower.find("orm") != std::string::npos) guess = TextureType::Specular;
                        else if (lower.find("emis") != std::string::npos || lower.find("glow") != std::string::npos) guess = TextureType::Emissive;
                        std::string fixed = FixTexturePath_FBX(path, std::filesystem::path(filePath), guess);
                        if (!fixed.empty()) dst.texturePaths[guess].push_back(fixed);
                    }
                }
            }
        };

    auto OldMatName = [&](uint32_t oldIdx)->std::string {
        if (oldIdx >= scene->mNumMaterials) return {};
        return SanitizeName(scene->mMaterials[oldIdx]->GetName().C_Str());
        };

    // ============================================================
    // [1] 메쉬 먼저 파싱: geometry + (oldMatIdx, oldMatName) 기록
    // ============================================================
    outModel.meshes.reserve(scene->mNumMeshes);
    std::vector<uint32_t>   meshOldMatIdx(scene->mNumMeshes, 0);
    std::vector<std::string> meshOldMatName(scene->mNumMeshes);

    for (unsigned m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh = scene->mMeshes[m];

        MeshData md{};
        md.name = SanitizeName(mesh->mName.C_Str());

        const uint32_t oldMat = (mesh->mMaterialIndex < scene->mNumMaterials) ? mesh->mMaterialIndex : 0;
        md.materialIndex = oldMat; // ★ 임시: oldMatIdx로 채워두고 나중에 교정
        meshOldMatIdx[m] = oldMat;
        meshOldMatName[m] = OldMatName(oldMat);

        // 정점
        md.positions.reserve(mesh->mNumVertices);
        if (mesh->HasNormals())               md.normals.reserve(mesh->mNumVertices);
        if (mesh->HasTextureCoords(0))        md.texcoords.reserve(mesh->mNumVertices);
        if (mesh->HasTangentsAndBitangents()) md.tangents.reserve(mesh->mNumVertices);

        for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
            md.positions.push_back({ _float(mesh->mVertices[i].x), _float(mesh->mVertices[i].y), _float(mesh->mVertices[i].z) });
            md.normals.push_back(mesh->HasNormals() ? _float3{ _float(mesh->mNormals[i].x), _float(mesh->mNormals[i].y), _float(mesh->mNormals[i].z) } : _float3{ 0,1,0 });
            md.texcoords.push_back(mesh->HasTextureCoords(0) ? _float2{ _float(mesh->mTextureCoords[0][i].x), _float(mesh->mTextureCoords[0][i].y) } : _float2{ 0,0 });
            if (mesh->HasTangentsAndBitangents())
            {
                md.tangents.push_back({ _float(mesh->mTangents[i].x), _float(mesh->mTangents[i].y), _float(mesh->mTangents[i].z) });
                md.binormals.push_back({ _float(mesh->mBitangents[i].x), _float(mesh->mBitangents[i].y), _float(mesh->mBitangents[i].z) });
            }
        }

        // 인덱스
        md.indices.reserve(mesh->mNumFaces * 3);
        for (unsigned f = 0; f < mesh->mNumFaces; ++f) {
            const aiFace& face = mesh->mFaces[f];
            for (unsigned k = 0; k < face.mNumIndices; ++k)
                md.indices.push_back(face.mIndices[k]);
        }

        // 본
        for (unsigned b = 0; b < mesh->mNumBones; ++b) {
            const aiBone* bone = mesh->mBones[b];
            MeshBone mb{};
            mb.name = SanitizeName(bone->mName.C_Str());
            mb.offsetMatrix = ToF4x4(bone->mOffsetMatrix);
            mb.weights.reserve(bone->mNumWeights);
            for (unsigned w = 0; w < bone->mNumWeights; ++w)
                mb.weights.push_back(VertexWeight{ bone->mWeights[w].mVertexId, bone->mWeights[w].mWeight });
            md.bones.push_back(std::move(mb));

            if (std::none_of(outModel.bones.begin(), outModel.bones.end(),
                [&](const BoneData& bd) { return bd.name == SanitizeName(bone->mName.C_Str()); })) {
                BoneData bd{};
                bd.name = SanitizeName(bone->mName.C_Str());
                bd.offsetMatrix = ToF4x4(bone->mOffsetMatrix);
                outModel.bones.push_back(std::move(bd));
            }
        }

        outModel.meshes.push_back(std::move(md));
    }

    // ============================================================
    // [2] 머티리얼 파싱: 벡터 생성 + 이름→인덱스 맵 구성
    // ============================================================
    outModel.materials.reserve(scene->mNumMaterials);
    for (unsigned i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* src = scene->mMaterials[i];

        MaterialData m{};
        m.name = SanitizeName(src->GetName().C_Str());
        if (m.name.empty()) m.name = "Mat_" + std::to_string(i);

        TryLoadAllTextureTypes(src, m);

        aiColor3D kd(1, 1, 1), ks(0, 0, 0), ke(0, 0, 0);
        src->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
        src->Get(AI_MATKEY_COLOR_SPECULAR, ks);
        src->Get(AI_MATKEY_COLOR_EMISSIVE, ke);
        m.diffuseColor = { (_float)kd.r, (_float)kd.g, (_float)kd.b, 1.0f };
        m.specularColor = { (_float)ks.r, (_float)ks.g, (_float)ks.b, 1.0f };
        m.emissiveColor = { (_float)ke.r, (_float)ke.g, (_float)ke.b, 1.0f };

        outModel.materials.push_back(std::move(m));
    }

    // 이름 → 인덱스 매핑 (정규화 키)
    std::unordered_map<std::string, uint32_t> nameToIdx;
    nameToIdx.reserve(outModel.materials.size());
    for (uint32_t i = 0; i < (uint32_t)outModel.materials.size(); ++i) {
        nameToIdx[SanitizeLower(outModel.materials[i].name)] = i;
    }

    // ============================================================
    // [3] 매쉬의 최종 머티리얼 인덱스 교정 (이름 우선, 인덱스 보조, 최후수단 append)
    // ============================================================
    auto AppendMaterialFromOld = [&](uint32_t oldIdx)->uint32_t {
        const aiMaterial* miss = (oldIdx < scene->mNumMaterials) ? scene->mMaterials[oldIdx] : nullptr;
        if (!miss) return 0;

        MaterialData fix{};
        fix.name = OldMatName(oldIdx);
        if (fix.name.empty()) fix.name = "Mat_" + std::to_string(oldIdx);

        TryLoadAllTextureTypes(miss, fix);
        aiColor3D kd(1, 1, 1), ks(0, 0, 0), ke(0, 0, 0);
        miss->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
        miss->Get(AI_MATKEY_COLOR_SPECULAR, ks);
        miss->Get(AI_MATKEY_COLOR_EMISSIVE, ke);
        fix.diffuseColor = { (_float)kd.r, (_float)kd.g, (_float)kd.b, 1.0f };
        fix.specularColor = { (_float)ks.r, (_float)ks.g, (_float)ks.b, 1.0f };
        fix.emissiveColor = { (_float)ke.r, (_float)ke.g, (_float)ke.b, 1.0f };

        uint32_t forced = (uint32_t)outModel.materials.size();
        outModel.materials.push_back(std::move(fix));
        nameToIdx[SanitizeLower(outModel.materials.back().name)] = forced;
        return forced;
        };

    for (size_t i = 0; i < outModel.meshes.size(); ++i)
    {
        auto& md = outModel.meshes[i];
        const uint32_t oldIdx = meshOldMatIdx[i];
        const std::string want = meshOldMatName[i];           // FBX가 의도한 이름(정규화 전)
        const std::string wantK = SanitizeLower(want);

        // 1) 이름으로 정확 매칭 → 최우선
        auto it = nameToIdx.find(wantK);
        if (it != nameToIdx.end()) {
            md.materialIndex = it->second;
            goto VERIFIED;
        }

        // 2) 이름 매칭 실패 → oldIdx로 보조 매칭
        if (oldIdx < outModel.materials.size()) {
            const std::string have = outModel.materials[oldIdx].name;
            if (!want.empty() && SanitizeLower(have) == wantK) {
                md.materialIndex = oldIdx;
                goto VERIFIED;
            }
        }

        // 3) 최후수단 → 해당 aiMaterial로 새 머티리얼을 즉시 append 후 교정
        {
            const uint32_t forced = AppendMaterialFromOld(oldIdx);
            md.materialIndex = forced;
            OutputDebugStringA(("[MatFix-ForcedAppend] mesh=" + md.name + " : appended '" + want + "'\n").c_str());
        }

    VERIFIED:
        // 방어
        if (!(md.materialIndex < outModel.materials.size())) {
            // 절대 오면 안 됨: 마지막 방어
            const uint32_t forced = AppendMaterialFromOld(oldIdx);
            md.materialIndex = forced;
        }

        // (선택) 검증 로그
        {
            char buf[512];
            sprintf_s(buf, "[MeshMatFinal] mesh=%s want='%s' -> newIdx=%u('%s')\n",
                md.name.c_str(), want.c_str(), md.materialIndex, outModel.materials[md.materialIndex].name.c_str());
            OutputDebugStringA(buf);
        }
    }

    // ============================================================
    // [4] 노드 트리
    // ============================================================
    int nodeCounter = 0;
    std::function<void(const aiNode*, NodeData&, int)> processNode;
    processNode = [&](const aiNode* node, NodeData& outNode, int parentIdx)
        {
            const int myIdx = nodeCounter++;
            outNode.name = (node->mName.length > 0) ? SanitizeName(node->mName.C_Str()) : "Root";
            outNode.parentIndex = parentIdx;
            outNode.transform = ToF4x4(node->mTransformation);
            if (!(outNode.transform._11 || outNode.transform._22 || outNode.transform._33))
                outNode.transform = IdentityF4x4();

            outNode.children.resize(node->mNumChildren);
            for (unsigned c = 0; c < node->mNumChildren; ++c)
                processNode(node->mChildren[c], outNode.children[c], myIdx);
        };
    processNode(scene->mRootNode, outModel.rootNode, -1);

    // ============================================================
    // [5] 애니메이션
    // ============================================================
    for (unsigned a = 0; a < scene->mNumAnimations; ++a)
    {
        const aiAnimation* anim = scene->mAnimations[a];

        AnimationData ad{};
        ad.name = (anim->mName.length > 0) ? SanitizeName(anim->mName.C_Str()) : "Anim_" + std::to_string(a);

        double tps = (anim->mTicksPerSecond != 0.0) ? anim->mTicksPerSecond : 30.0;
        if (tps > 480.0) tps = 30.0;
        ad.ticksPerSecond = (float)tps;
        ad.duration = (float)anim->mDuration;

        ad.channels.reserve(anim->mNumChannels);
        for (unsigned c = 0; c < anim->mNumChannels; ++c)
        {
            const aiNodeAnim* ch = anim->mChannels[c];
            ChannelData cd{}; cd.nodeName = SanitizeName(ch->mNodeName.C_Str());

            for (unsigned k = 0; k < ch->mNumPositionKeys; ++k)
                cd.positionKeys.push_back({ (float)ch->mPositionKeys[k].mTime,
                    { (float)ch->mPositionKeys[k].mValue.x, (float)ch->mPositionKeys[k].mValue.y, (float)ch->mPositionKeys[k].mValue.z } });

            for (unsigned k = 0; k < ch->mNumRotationKeys; ++k) {
                _float4 q{ (float)ch->mRotationKeys[k].mValue.x, (float)ch->mRotationKeys[k].mValue.y, (float)ch->mRotationKeys[k].mValue.z, (float)ch->mRotationKeys[k].mValue.w };
                NormalizeQuat(q);
                cd.rotationKeys.push_back({ (float)ch->mRotationKeys[k].mTime, q });
            }

            for (unsigned k = 0; k < ch->mNumScalingKeys; ++k)
                cd.scalingKeys.push_back({ (float)ch->mScalingKeys[k].mTime,
                    { (float)ch->mScalingKeys[k].mValue.x, (float)ch->mScalingKeys[k].mValue.y, (float)ch->mScalingKeys[k].mValue.z } });

            ad.channels.push_back(std::move(cd));
        }

        outModel.animations.push_back(std::move(ad));
    }

    PruneModelBonesToAnimSet(outModel, "root", false, 4);

    // 🔹 정적인(변화 없는) 애니메이션도 필터링
    AnimPruneThreshold th{};
    th.requireRootMotion = false;
    th.rootName = "root";
    PruneStaticAnimations(outModel, th);

    return true;
}


bool IEHelper::ExportModel(const std::string& filePath, const ModelData& model)
{
    // ============= [1] 전체 모델 내보내기 =============
    {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;

        auto WriteU32 = [&](uint32_t v) { file.write((char*)&v, 4); };
        auto WriteF4x4 = [&](const _float4x4& m) { file.write((char*)&m, sizeof(_float4x4)); };
        auto WriteF4 = [&](const _float4& v) { file.write((char*)&v, sizeof(_float4)); };
        auto WriteStr = [&](const std::string& s) {
            _uint n = (_uint)s.size(); file.write((char*)&n, 4);
            if (n) file.write(s.data(), n);
            };
        auto WriteVector = [&](auto& v) {
            _uint n = (_uint)v.size(); file.write((char*)&n, 4);
            if (n) file.write((char*)v.data(), sizeof(v[0]) * n);
            };

        // [NEW] 헤더: "MDL2" + version(2)
        const char magic[4] = { 'M','D','L','2' };
        file.write(magic, 4);
        WriteU32(2u); // version = 2

        // 1. Meshes
        _uint numMeshes = (_uint)model.meshes.size();
        file.write((char*)&numMeshes, 4);
        for (auto& mesh : model.meshes)
        {
            WriteStr(mesh.name);
            file.write((char*)&mesh.materialIndex, 4);
            WriteVector(mesh.positions);
            WriteVector(mesh.normals);
            WriteVector(mesh.texcoords);
            WriteVector(mesh.tangents);
            WriteVector(mesh.indices);

            _uint numBones = (_uint)mesh.bones.size();
            file.write((char*)&numBones, 4);
            for (auto& b : mesh.bones)
            {
                WriteStr(b.name);
                _uint w = (_uint)b.weights.size();
                file.write((char*)&w, 4);
                if (w) file.write((char*)b.weights.data(), sizeof(VertexWeight) * w);
                WriteF4x4(b.offsetMatrix);
            }
        }

        // 2. Materials
        _uint numMat = (_uint)model.materials.size();
        file.write((char*)&numMat, 4);
        for (auto& m : model.materials)
        {
            WriteStr(m.name);

            // (a) 텍스처 경로들 (기존과 동일)
            for (int t = 0; t < (int)TextureType::End; ++t)
            {
                _uint numTex = (_uint)m.texturePaths[t].size();
                file.write((char*)&numTex, 4);
                for (auto& p : m.texturePaths[t])
                    WriteStr(p);
            }

            // (b) [NEW] 컬러 3종 (_float4): diffuse/specular/emissive
            WriteF4(m.diffuseColor);
            WriteF4(m.specularColor);
            WriteF4(m.emissiveColor);
        }

        // 3. Node 트리
        std::function<void(const NodeData&)> WNode = [&](const NodeData& n)
            {
                WriteStr(n.name);
                file.write((char*)&n.parentIndex, 4);
                WriteF4x4(n.transform);
                _uint c = (_uint)n.children.size();
                file.write((char*)&c, 4);
                for (auto& ch : n.children) WNode(ch);
            };
        WNode(model.rootNode);

        // 4. Animations
        _uint numAnim = (_uint)model.animations.size();
        file.write((char*)&numAnim, 4);
        for (auto& a : model.animations)
        {
            WriteStr(a.name);
            file.write((char*)&a.duration, 4);
            file.write((char*)&a.ticksPerSecond, 4);

            _uint c = (_uint)a.channels.size();
            file.write((char*)&c, 4);
            for (auto& ch : a.channels)
            {
                WriteStr(ch.nodeName);

                auto WK = [&](auto& v) {
                    _uint n = (_uint)v.size();
                    file.write((char*)&n, 4);
                    if (n) file.write((char*)v.data(), sizeof(v[0]) * n);
                    };
                WK(ch.positionKeys);
                WK(ch.rotationKeys);
                WK(ch.scalingKeys);
            }
        }

        // 5. Global Bones
        _uint nb = (_uint)model.bones.size();
        file.write((char*)&nb, 4);
        for (auto& b : model.bones)
        {
            WriteStr(b.name);
            WriteF4x4(b.offsetMatrix);
        }

        // 6. 원본 경로
        WriteStr(model.modelDataFilePath);

        file.close();
    }

    // ============= [2] 각 Mesh별 개별 bin 내보내기 =============
    std::filesystem::path basePath = std::filesystem::path(filePath).parent_path();
    std::string baseName = std::filesystem::path(filePath).stem().string();

    for (size_t i = 0; i < model.meshes.size(); ++i)
    {
        std::string meshFile = (basePath / (baseName + "_Mesh" + std::to_string(i) + ".bin")).string();
        std::ofstream mfile(meshFile, std::ios::binary);
        if (!mfile.is_open())
            continue;

        const auto& mesh = model.meshes[i];

        auto WriteU32 = [&](uint32_t v) { mfile.write((char*)&v, 4); };
        auto WriteF4x4 = [&](const _float4x4& m) { mfile.write((char*)&m, sizeof(_float4x4)); };
        auto WriteF4 = [&](const _float4& v) { mfile.write((char*)&v, sizeof(_float4)); };
        auto WriteStr = [&](const std::string& s) {
            _uint n = (_uint)s.size(); mfile.write((char*)&n, 4);
            if (n) mfile.write(s.data(), n);
            };
        auto W = [&](auto& v) { _uint n = (_uint)v.size(); mfile.write((char*)&n, 4); if (n) mfile.write((char*)v.data(), sizeof(v[0]) * n); };

        // [NEW] 헤더: "MDL2" + version(2)
        const char magic[4] = { 'M','D','L','2' };
        mfile.write(magic, 4);
        WriteU32(2u);

        // --- Meshes (단일)
        _uint numMeshes = 1;
        mfile.write((char*)&numMeshes, 4);
        {
            WriteStr(mesh.name);
            _uint fixedMatIdx = 0; // 단일 파일 안에서는 0번만 사용
            mfile.write((char*)&fixedMatIdx, 4);

            W(mesh.positions); W(mesh.normals); W(mesh.texcoords); W(mesh.tangents); W(mesh.indices);

            _uint numBones = (_uint)mesh.bones.size();
            mfile.write((char*)&numBones, 4);
            for (auto& b : mesh.bones)
            {
                WriteStr(b.name);
                _uint w = (_uint)b.weights.size(); mfile.write((char*)&w, 4);
                if (w) mfile.write((char*)b.weights.data(), sizeof(VertexWeight) * w);
                WriteF4x4(b.offsetMatrix);
            }
        }

        // --- Materials (메쉬가 참조하는 하나만 포함)
        _uint numMat = 0;
        if (mesh.materialIndex < model.materials.size())
        {
            numMat = 1;
            mfile.write((char*)&numMat, 4);

            const auto& mat = model.materials[mesh.materialIndex];
            WriteStr(mat.name);

            // (a) 텍스처 경로들
            for (int t = 0; t < (int)TextureType::End; ++t)
            {
                _uint numTex = (_uint)mat.texturePaths[t].size();
                mfile.write((char*)&numTex, 4);
                for (auto& p : mat.texturePaths[t]) WriteStr(p);
            }
            // (b) [NEW] 컬러 3종
            WriteF4(mat.diffuseColor);
            WriteF4(mat.specularColor);
            WriteF4(mat.emissiveColor);
        }
        else
        {
            mfile.write((char*)&numMat, 4);
        }

        // --- Node (간단 루트만)
        {
            NodeData root{};
            root.name = "Root";
            root.parentIndex = -1;
            root.transform = IdentityF4x4();
            _uint c = 0;

            WriteStr(root.name);
            mfile.write((char*)&root.parentIndex, 4);
            WriteF4x4(root.transform);
            mfile.write((char*)&c, 4);
        }

        // --- Animations (개별 메쉬 bin에는 없음)
        _uint numAnim = 0;
        mfile.write((char*)&numAnim, 4);

        // --- Global Bones (해당 메쉬 본만)
        _uint nb = (_uint)mesh.bones.size();
        mfile.write((char*)&nb, 4);
        for (auto& b : mesh.bones)
        {
            WriteStr(b.name);
            WriteF4x4(b.offsetMatrix);
        }

        // 원본 경로
        WriteStr(model.modelDataFilePath);

        char buf[256];
        sprintf_s(buf, "[Export] %s : vtx=%zu, idx=%zu, matIdx=%d\n",
            mesh.name.c_str(), mesh.positions.size(), mesh.indices.size(), mesh.materialIndex);
        OutputDebugStringA(buf);

        mfile.close();
    }

    return true;
}
