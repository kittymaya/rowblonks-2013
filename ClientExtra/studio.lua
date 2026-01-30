print("hi")

local InsertService = game:GetService("InsertService")
InsertService:SetTrustLevel(0)
InsertService:SetBaseSetsUrl("http://sets.pizzaboxer.xyz/Game/Tools/InsertAsset.ashx?nsets=10&type=base")
InsertService:SetUserSetsUrl("http://sets.pizzaboxer.xyz/Game/Tools/InsertAsset.ashx?nsets=20&type=user&userid=%d")
InsertService:SetCollectionUrl("http://sets.pizzaboxer.xyz/Game/Tools/InsertAsset.ashx?sid=%d")
InsertService:SetFreeModelUrl("http://sets.pizzaboxer.xyz/Game/Tools/InsertAsset.ashx?type=fm&q=%s&pg=%d&rs=%d")
InsertService:SetFreeDecalUrl("http://sets.pizzaboxer.xyz/Game/Tools/InsertAsset.ashx?type=fd&q=%s&pg=%d&rs=%d")
InsertService:SetAssetUrl("https://assetdelivery.roblox.com/v1/asset/?id=%d")
InsertService:SetAssetVersionUrl("https://assetdelivery.roblox.com/v1/asset/?assetversionid=%d")

maya.AddLocalCoreScript("StarterScript", game:GetService("ScriptContext"))
