function helpers_setComponent(enttId, componentName, args)
    scene.SetComponent(enttId, componentName, table.unpack(args))
end