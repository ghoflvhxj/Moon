DirectX 11을 이용해 간단한 엔진을 만드는 것을 목표로 하는 프로젝트입니다.

주요 특징은 다음과 같습니다.
- 리플렉션에 기반한 에디터 기능 (편집, PIE)
![AssetEdit](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/AssetEdit.webp)
![PIE](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/PIE.webp)
- 디퍼드 렌더링
![Def](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/DeferredRendering.JPG)
![Def2](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/DeferredRendering2.webp)
- Effect 프레임워크가 아닌 HLSL 리플렉션 직접 구현
- Cascade Shadow 구현
![CascadeShadow](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/CascadeShadow.gif)
- FBX SDK를 이용하여 3D 모델링 로드, 애니메이션 재생
- NVIDIA PhysX를 이용한 물리 시뮬레이션 -> JoltPhysics로 변경 되었습니다.
![PhysX](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/PhysX.gif)
![JoltPhysics](https://github.com/ghoflvhxj/Moon/blob/main/ReadmeResource/JoltPhysics.webp)
