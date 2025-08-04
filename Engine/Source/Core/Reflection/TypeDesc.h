#pragma once

/*
클래스, 구조체 같은 커스텀 자료형을 설명하는 구조체
이름, 부모 클래스, 사이즈, 어떤 멤버들을 가지고 있는지 등등
외부 라이브러리의 타입을 설명하려면 GetTypeDesc를 특수화 해주면 됨.
*/

struct FPropertyDesc;

struct FTypeDesc
{
	const FTypeDesc* Parent = nullptr;
	std::string Name;
	size_t Size;

    // 멤버 정의를 담음
	std::vector<FPropertyDesc*> Properties;
};

// 외부 타입의 Desc 생성을 위한 템플릿
template <class T>
static const FTypeDesc* GetTypeDesc()
{
    return T::GetTypeDescStatic();
}