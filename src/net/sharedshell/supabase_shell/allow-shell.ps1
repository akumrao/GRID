Remove-NetFirewallRule -DisplayName "Supabase Shell *" -ErrorAction SilentlyContinue

New-NetFirewallRule -DisplayName "Supabase Shell - Inbound Allow All" -Direction Inbound -Program ((Resolve-Path "./shell.exe").Path) -Action Allow -Protocol Any -Profile Any

New-NetFirewallRule -DisplayName "Supabase Shell - Outbound Allow All" -Direction Outbound -Program ((Resolve-Path "./shell.exe").Path) -Action Allow -Protocol Any -Profile Any
