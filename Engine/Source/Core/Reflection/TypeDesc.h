#pragma once

// 클래스, 구조체와 같은 커스텀 타입에 대한 정보를 담음
// 이름, 부모 클래스, 사이즈, 어떤 멤버들을 가지고 있는지 등등
struct FTypeDesc
{
	const FTypeDesc* Parent = nullptr;
	std::string Name;
	size_t Size;
	std::vector<FPropertyDesc*> Properties;
};
