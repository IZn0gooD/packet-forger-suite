# Auto-complétion Bash pour packet-forger
# Installation: source completions/packet-forger.bash
# Ou: sudo cp completions/packet-forger.bash /etc/bash_completion.d/

_packet_forger() {
    local cur prev opts modules
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    
    # Modules disponibles
    modules="core payload lab research"
    
    # Options globales (disponibles partout)
    local global_opts="--help --version --verbose -h -v -V"
    
    # Si on est au premier argument (module)
    if [ ${COMP_CWORD} -eq 1 ]; then
        COMPREPLY=($(compgen -W "${modules} ${global_opts}" -- ${cur}))
        return 0
    fi
    
    local module="${COMP_WORDS[1]}"
    
    # Vérifier si on a tapé une option globale
    if [[ "${module}" == --* ]] || [[ "${module}" == -* ]]; then
        COMPREPLY=($(compgen -W "${global_opts}" -- ${cur}))
        return 0
    fi
    
    # Complétion selon le module
    case "${module}" in
        core)
            if [ ${COMP_CWORD} -eq 2 ]; then
                COMPREPLY=($(compgen -W "send --help -h ${global_opts}" -- ${cur}))
            elif [ "${COMP_WORDS[2]}" == "send" ]; then
                # On est dans le contexte "core send", proposer toutes les options
                if [ "${prev}" == "--payload" ]; then
                    # Complétion des fichiers .bin
                    COMPREPLY=($(compgen -f -X '!*.bin' -- ${cur}))
                elif [ "${prev}" == "--target" ] || [ "${prev}" == "--source-ip" ]; then
                    # Pas de complétion pour les IPs
                    return 0
                elif [ "${prev}" == "--port" ] || [ "${prev}" == "--source-port" ] || \
                     [ "${prev}" == "--ttl" ] || [ "${prev}" == "--window" ]; then
                    # Pas de complétion pour les ports/numéros
                    return 0
                elif [ "${prev}" == "--transport-profile" ]; then
                    COMPREPLY=($(compgen -W "linux windows custom" -- ${cur}))
                elif [ "${prev}" == "--interface" ]; then
                    # Complétion des interfaces réseau
                    COMPREPLY=($(compgen -W "$(ip -o link show 2>/dev/null | awk -F': ' '{print $2}')" -- ${cur}))
                else
                    # Proposer toutes les options core (incluant les options globales)
                    local core_opts="--payload --target --port --transport-profile --fragment --stealth --source-ip --source-port --ttl --window --interface --help -h ${global_opts}"
                    COMPREPLY=($(compgen -W "${core_opts}" -- ${cur}))
                fi
            elif [ "${prev}" == "--payload" ]; then
                # Complétion des fichiers .bin
                COMPREPLY=($(compgen -f -X '!*.bin' -- ${cur}))
            elif [ "${prev}" == "--target" ] || [ "${prev}" == "--source-ip" ]; then
                # Pas de complétion pour les IPs
                return 0
            elif [ "${prev}" == "--port" ] || [ "${prev}" == "--source-port" ] || \
                 [ "${prev}" == "--ttl" ] || [ "${prev}" == "--window" ]; then
                # Pas de complétion pour les ports/numéros
                return 0
            elif [ "${prev}" == "--transport-profile" ]; then
                COMPREPLY=($(compgen -W "linux windows custom" -- ${cur}))
            elif [ "${prev}" == "--interface" ]; then
                # Complétion des interfaces réseau
                COMPREPLY=($(compgen -W "$(ip -o link show 2>/dev/null | awk -F': ' '{print $2}')" -- ${cur}))
            else
                # Options du module core (incluant les options globales)
                local core_opts="--payload --target --port --transport-profile --fragment --stealth --source-ip --source-port --ttl --window --interface --help -h ${global_opts}"
                COMPREPLY=($(compgen -W "${core_opts}" -- ${cur}))
            fi
            ;;
        payload)
            if [ ${COMP_CWORD} -eq 2 ]; then
                COMPREPLY=($(compgen -W "build --help -h ${global_opts}" -- ${cur}))
            elif [ "${COMP_WORDS[2]}" == "build" ]; then
                # On est dans le contexte "payload build"
                if [ "${prev}" == "build" ]; then
                    # Après "build", proposer les plateformes
                    COMPREPLY=($(compgen -W "linux windows" -- ${cur}))
                elif [ "${prev}" == "linux" ] || [ "${prev}" == "windows" ]; then
                    # Après la plateforme, proposer les types
                    COMPREPLY=($(compgen -W "reverse bind" -- ${cur}))
                elif [ "${prev}" == "--lhost" ] || [ "${prev}" == "--source-ip" ]; then
                    # Pas de complétion pour les IPs
                    return 0
                elif [ "${prev}" == "--lport" ]; then
                    # Pas de complétion pour les ports
                    return 0
                elif [ "${prev}" == "--output" ]; then
                    # Complétion des fichiers
                    COMPREPLY=($(compgen -f -- ${cur}))
                else
                    # Proposer toutes les options payload (incluant les options globales)
                    local payload_opts="--lhost --lport --output --polymorphic --asm --encode-xor --help -h ${global_opts}"
                    COMPREPLY=($(compgen -W "${payload_opts}" -- ${cur}))
                fi
            fi
            ;;
        lab)
            if [ ${COMP_CWORD} -eq 2 ]; then
                COMPREPLY=($(compgen -W "list show test server --help -h ${global_opts}" -- ${cur}))
            elif [ "${prev}" == "show" ] || [ "${prev}" == "test" ]; then
                COMPREPLY=($(compgen -W "CVE-2015-1635 CVE-2020-0796" -- ${cur}))
            elif [ "${prev}" == "server" ]; then
                COMPREPLY=($(compgen -W "http smb" -- ${cur}))
            else
                # Options globales disponibles après les actions
                COMPREPLY=($(compgen -W "${global_opts}" -- ${cur}))
            fi
            ;;
        research)
            if [ ${COMP_CWORD} -eq 2 ]; then
                COMPREPLY=($(compgen -W "list show --help -h ${global_opts}" -- ${cur}))
            elif [ "${prev}" == "show" ]; then
                COMPREPLY=($(compgen -W "ASLR DEP_NX DEP NX STACK_CANARY CANARY ROP_TEMPLATES ROP MEMORY_CORRELATION" -- ${cur}))
            else
                # Options globales disponibles après les actions
                COMPREPLY=($(compgen -W "${global_opts}" -- ${cur}))
            fi
            ;;
        *)
            # Si le module n'est pas reconnu, proposer les modules
            COMPREPLY=($(compgen -W "${modules} ${global_opts}" -- ${cur}))
            ;;
    esac
    
    return 0
}

complete -F _packet_forger packet-forger

