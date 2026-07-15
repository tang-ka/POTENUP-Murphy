#include "SheetSyncerSettingsCustomization.h"
#include "SheetSyncerSettings.h"
#include "GoogleSheetsSyncer.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyEditorModule.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"

TSharedRef<IDetailCustomization> FSheetSyncerSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FSheetSyncerSettingsCustomization());
}

void FSheetSyncerSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Categories 프로퍼티 숨기기
	TSharedRef<IPropertyHandle> CategoriesHandle =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USheetSyncerSettings, Categories));
	DetailBuilder.HideProperty(CategoriesHandle);

	USheetSyncerSettings* Settings = GetMutableDefault<USheetSyncerSettings>();
	if (!Settings)
	{
		return;
	}

	// Sync All 버튼
	IDetailCategoryBuilder& ActionsCategory =
		DetailBuilder.EditCategory("Actions", FText::FromString("Actions"));

	ActionsCategory.AddCustomRow(FText::FromString("SyncAll"))
	               .WholeRowContent()
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("Sync All"))
		.OnClicked_Lambda([]()
		{
			UGoogleSheetsSyncer::SyncAll();
			return FReply::Handled();
		})
	];

	// Categories 섹션
	IDetailCategoryBuilder& CategoriesCategory =
		DetailBuilder.EditCategory("Categories", FText::FromString("Categories"));

	// Categories 추가 버튼
	CategoriesCategory.AddCustomRow(FText::FromString("AddCategory"))
	                  .WholeRowContent()
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.Text(FText::FromString("+ Add Category"))
		.OnClicked_Lambda([&DetailBuilder, Settings]()
		{
			FSheetSyncerCategory NewCategory;
			NewCategory.CategoryName = TEXT("New Category");
			Settings->Categories.Add(NewCategory);
			Settings->SaveConfig();
			DetailBuilder.ForceRefreshDetails();
			return FReply::Handled();
		})
	];

	// 각 카테고리 표시
	for (int32 i = 0; i < Settings->Categories.Num(); i++)
	{
		FSheetSyncerCategory& Category = Settings->Categories[i];
		FString CategoryName = Category.CategoryName.IsEmpty()
			                       ? FString::Printf(TEXT("Category %d"), i)
			                       : Category.CategoryName;

		IDetailCategoryBuilder& CatBuilder =
			DetailBuilder.EditCategory(
				*FString::Printf(TEXT("Cat_%d"), i),
				FText::FromString(CategoryName));

		// CategoryName 편집
		TSharedPtr<IPropertyHandle> CategoryHandle = CategoriesHandle->GetChildHandle(i);
		if (CategoryHandle.IsValid())
		{
			TSharedPtr<IPropertyHandle> NameHandle =
				CategoryHandle->GetChildHandle(
					GET_MEMBER_NAME_CHECKED(FSheetSyncerCategory, CategoryName));
			if (NameHandle.IsValid())
			{
				NameHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateLambda([&DetailBuilder]()
					{
						DetailBuilder.ForceRefreshDetails();
					}));

				CatBuilder.AddProperty(NameHandle.ToSharedRef());
			}

			// Entries 편집
			TSharedPtr<IPropertyHandle> EntriesHandle =
				CategoryHandle->GetChildHandle(
					GET_MEMBER_NAME_CHECKED(FSheetSyncerCategory, Entries));
			if (EntriesHandle.IsValid())
			{
				CatBuilder.AddProperty(EntriesHandle.ToSharedRef());
			}
		}

		// 카테고리별 Sync 버튼
		int32 CategoryIndex = i;
		CatBuilder.AddCustomRow(FText::FromString("SyncCategory"))
		          .WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(FText::FromString(FString::Printf(TEXT("Sync %s"), *CategoryName)))
				.OnClicked_Lambda([CategoryIndex]()
				{
					UGoogleSheetsSyncer::SyncCategory(CategoryIndex);
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Remove")))
				.OnClicked_Lambda([&DetailBuilder, Settings, CategoryIndex]()
				{
					Settings->Categories.RemoveAt(CategoryIndex);
					Settings->SaveConfig();
					DetailBuilder.ForceRefreshDetails();
					return FReply::Handled();
				})
			]
		];
	}
}
