# Bus Modules

Neko Bus is the bus module in NekoLauncher that manages global resources such as thread pools, event buses, and configuration managers.

It provides singleton-style global access interfaces, allowing modules to easily share resources without passing instances.

## Testing

Neko Bus does not undergo unit testing, as it is merely a wrapper for resource management and does not contain complex logic that requires testing.
Testing for each resource should be performed within their respective modules, such as the thread pool or event system.