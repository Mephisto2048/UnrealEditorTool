#pragma once
void Print()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 8.f, FColor::Cyan, "debug");
	}
}

void ShowNotify()
{

}