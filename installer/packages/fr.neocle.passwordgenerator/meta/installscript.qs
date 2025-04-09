function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();

    component.addOperation("CreateShortcut", 
        "@TargetDir@/PasswordGenerator.exe",       
        "@DesktopDir@/PasswordGenerator.lnk",        
        "workingDirectory=@TargetDir@",             
        "iconPath=@TargetDir@/icon.ico",    
        "iconId=0",                                   
        "description=Start Password Generator"
    );
};