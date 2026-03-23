import React, { useState, useEffect } from 'react';
import Home from '../pages/home';
import Catalogue from '../pages/catalogue';
import Account from '../pages/account';
import Management from '../pages/managment';
import { api } from '../api';

export default function Layout() {
    const [expanded, setExpanded] = useState(true);
    const [activeTab, setActiveTab] = useState('home');
    const [showAccountMenu, setShowAccountMenu] = useState(false);

    const [isAuthenticated, setIsAuthenticated] = useState(false);
    const [user, setUser] = useState(null);
    const [showAuthModal, setShowAuthModal] = useState(false);
    const [authMode, setAuthMode] = useState('login');
    // Добавлено поле phone
    const [authForm, setAuthForm] = useState({ username: '', password: '', nickname: '', phone: '' });
    const [authError, setAuthError] = useState('');
    const [isLoading, setIsLoading] = useState(true);

    // Проверка сессии при загрузке
    useEffect(() => {
        api.getProfile().then(res => {
            setUser(res.user);
            setIsAuthenticated(true);
        }).catch(() => {
        }).finally(() => {
            setIsLoading(false);
        });
    },[]);

    const handleAuthSubmit = async (e) => {
        e.preventDefault();
        setAuthError('');
        try {
            if (authMode === 'register') {
                // Добавлен phone в отправку
                await api.register({
                    username: authForm.username,
                    password: authForm.password,
                    nickname: authForm.nickname,
                    phone: authForm.phone
                });
            } else {
                const profileRes = await api.login({ username: authForm.username, password: authForm.password });
                setUser(profileRes.user);
            }
            setIsAuthenticated(true);
            setShowAuthModal(false);
            // Сброс формы теперь включает phone
            setAuthForm({ username: '', password: '', nickname: '', phone: '' });
        } catch (err) {
            setAuthError(err.message);
        }
    };

    const handleLogout = async () => {
        try {
            await api.logout();
        } catch (e) {
            console.error("Ошибка при выходе:", e);
        }
        setIsAuthenticated(false);
        setUser(null);
        setShowAccountMenu(false);
        setActiveTab('home');
    };

    const navItems =[
        { id: 'home', label: 'Главная', icon: 'home' },
        { id: 'catalog', label: 'Каталог', icon: 'pets' },
        ...(user?.access_level === 1 ? [{ id: 'management', label: 'Менеджмент', icon: 'work' }] :[])
    ];

    if (isLoading) {
        return (
            <div className="flex h-screen items-center justify-center bg-[#FFE1E8] font-m3 overflow-hidden">
                <div className="flex flex-col items-center gap-6 animate-in fade-in zoom-in duration-500">
                    <div className="relative flex items-center justify-center w-32 h-32 bg-white rounded-full shadow-xl shadow-[#F6828C]/20 animate-bounce">
                        <span className="material-symbols-rounded text-6xl text-[#F6828C]">pets</span>
                    </div>
                    <h1 className="font-seymour text-4xl sm:text-5xl text-[#F6828C] tracking-wide drop-shadow-sm">
                        Rent'A'Purr
                    </h1>
                    <div className="flex gap-2">
                        <div className="w-3 h-3 bg-[#F6828C] rounded-full animate-bounce" style={{ animationDelay: '0ms' }} />
                        <div className="w-3 h-3 bg-[#F6828C] rounded-full animate-bounce" style={{ animationDelay: '150ms' }} />
                        <div className="w-3 h-3 bg-[#F6828C] rounded-full animate-bounce" style={{ animationDelay: '300ms' }} />
                    </div>
                </div>
            </div>
        );
    }

    return (
        <div className="flex h-screen bg-[#f3f4f6] font-m3 text-[#1D192B] overflow-hidden p-3 gap-3">
            <nav className={`transition-all duration-300 ease-[cubic-bezier(0.2,0,0,1)] flex flex-col py-4 relative z-20 ${expanded ? 'w-[280px]' : 'w-[72px]'}`}>
                <div className={`flex items-center mb-6 transition-all duration-300 ease-[cubic-bezier(0.2,0,0,1)] px-2 ${expanded ? 'justify-between' : 'justify-center'}`}>
                    <div className={`overflow-hidden transition-all duration-300 flex items-center ${expanded ? 'w-auto opacity-100 ml-3' : 'w-0 opacity-0'}`}>
                        <span className="font-spray text-2xl text-[#F6828C] tracking-wide whitespace-nowrap drop-shadow-sm select-none leading-none mt-1">Rent'A'Purr</span>
                    </div>
                    <button onClick={() => setExpanded(!expanded)} className="w-12 h-12 flex items-center justify-center hover:bg-black/5 rounded-full transition-all duration-200 text-[#49454F] hover:text-[#1D192B] shrink-0">
                        <span className="material-symbols-rounded text-[24px]">{expanded ? 'menu_open' : 'menu'}</span>
                    </button>
                </div>

                <div className={`mb-6 flex justify-center w-full ${expanded ? 'px-3' : 'px-2'}`}>
                    <button onClick={() => setActiveTab('catalog')} className={`bg-[#F6828C] text-white flex items-center overflow-hidden transition-all duration-300 ease-[cubic-bezier(0.2,0,0,1)] hover:shadow-lg hover:bg-[#F36B77] active:scale-[0.98] ${expanded ? 'w-full rounded-[20px] h-16' : 'w-14 h-14 rounded-[16px]'}`}>
                        <div className={`h-full flex items-center justify-center shrink-0 transition-all duration-300 ${expanded ? 'w-[64px]' : 'w-14'}`}><span className="material-symbols-rounded" style={{ fontVariationSettings: "'FILL' 1, 'wght' 700" }}>favorite</span></div>
                        <div className="flex-1 whitespace-nowrap text-left pr-6"><span className="font-bold text-[16px]">Каталог</span></div>
                    </button>
                </div>

                <div className={`flex flex-col gap-1.5 flex-1 overflow-y-auto no-scrollbar ${expanded ? 'px-3' : 'px-2'}`}>
                    {navItems.map(item => (
                        <button key={item.id} onClick={() => setActiveTab(item.id)} className={`relative flex items-center h-[52px] w-full rounded-full overflow-hidden transition-all duration-200 group ${activeTab === item.id ? 'text-[#711A42]' : 'text-[#49454F] hover:bg-black/5'}`}>
                            <div className={`absolute inset-0 bg-[#FFE1E8] rounded-full transition-opacity duration-200 ${activeTab === item.id ? 'opacity-100' : 'opacity-0'}`} />
                            <div className="relative flex items-center w-full h-full z-10">
                                <div className="w-[56px] h-full flex items-center justify-center shrink-0"><span className="material-symbols-rounded filled text-[22px]">{item.icon}</span></div>
                                <div className="flex-1 whitespace-nowrap text-left pr-4"><span className={`text-[15px] ${activeTab === item.id ? 'font-bold' : 'font-medium'}`}>{item.label}</span></div>
                            </div>
                        </button>
                    ))}
                </div>

                <div className={`mt-auto pt-4 relative ${expanded ? 'px-3' : 'px-2'}`}>
                    {isAuthenticated ? (
                        <>
                            <div className={`flex transition-all duration-300 ease-[cubic-bezier(0.2,0,0,1)] ${showAccountMenu && expanded ? 'gap-2' : 'gap-[2px]'}`}>
                                <button onClick={() => { setActiveTab('account'); setShowAccountMenu(false); }} className={`relative flex items-center h-14 overflow-hidden transition-all duration-300 ${expanded ? 'flex-1' : 'w-14'} ${expanded ? (showAccountMenu ? 'rounded-l-[28px] rounded-r-[24px]' : 'rounded-l-[28px] rounded-r-[6px]') : 'rounded-[28px]'} ${activeTab === 'account' ? 'bg-[#FFE1E8] text-[#711A42]' : 'bg-[#E1E1E8] hover:bg-[#D5D5DC] text-[#49454F]'}`}>
                                    <div className="w-[56px] h-full flex items-center justify-center shrink-0"><span className="material-symbols-rounded text-[28px]">account_circle</span></div>
                                    <div className="flex-1 flex flex-col items-start justify-center whitespace-nowrap"><span className="font-semibold text-[14px] truncate w-full pr-2 text-left">{user?.nickname}</span></div>
                                </button>
                                {expanded && (
                                    <button onClick={() => setShowAccountMenu(!showAccountMenu)} className={`w-[44px] flex items-center justify-center shrink-0 transition-all duration-300 text-[#49454F] bg-[#E1E1E8] hover:bg-[#D5D5DC] ${showAccountMenu ? 'rounded-[24px] bg-[#D5D5DC]' : 'rounded-r-[28px] rounded-l-[6px]'}`}>
                                        <span className={`material-symbols-rounded text-[24px] transition-transform duration-300 ${showAccountMenu ? '-rotate-180' : 'rotate-0'}`}>keyboard_arrow_down</span>
                                    </button>
                                )}
                            </div>
                            {expanded && (
                                <div className={`absolute bottom-full left-3 right-3 mb-3 bg-white rounded-[24px] shadow-xl border border-[#E7E0EC] py-2 z-50 flex flex-col gap-0.5 origin-bottom transition-all duration-300 ${showAccountMenu ? 'opacity-100 scale-100 translate-y-0 pointer-events-auto' : 'opacity-0 scale-95 translate-y-4 pointer-events-none'}`}>
                                    <div className="px-2"><button onClick={handleLogout} className="w-full flex items-center h-[44px] rounded-[8px] hover:bg-[#F6828C]/10 text-[#F6828C] transition-colors"><div className="w-[44px] flex items-center justify-center shrink-0"><span className="material-symbols-rounded text-[20px]">logout</span></div><span className="text-[14px] font-semibold">Выйти</span></button></div>
                                </div>
                            )}
                        </>
                    ) : (
                        <button onClick={() => setShowAuthModal(true)} className={`relative flex items-center h-14 rounded-[28px] overflow-hidden transition-all duration-300 bg-[#E1E1E8] hover:bg-[#D5D5DC] text-[#49454F] ${expanded ? 'w-full' : 'w-14'}`}>
                            <div className="w-[56px] h-full flex items-center justify-center shrink-0"><span className="material-symbols-rounded text-[28px]">login</span></div>
                            <div className="flex-1 flex flex-col items-start justify-center whitespace-nowrap"><span className="font-semibold text-[14px] pr-2 text-left">Войти</span></div>
                        </button>
                    )}
                </div>
            </nav>

            <main className="flex-1 bg-white rounded-[32px] overflow-y-auto shadow-sm relative transition-all duration-300 no-scrollbar">
                <div key={activeTab} className="h-full animate-tab">
                    {activeTab === 'home' && <Home />}
                    {activeTab === 'catalog' && <Catalogue />}
                    {activeTab === 'management' && <Management />}
                    {activeTab === 'account' && <Account />}
                </div>
            </main>

            {showAuthModal && (
                <div className="fixed inset-0 bg-black/40 backdrop-blur-sm z-[100] flex items-center justify-center p-4 animate-in fade-in duration-200">
                    <div className="bg-white rounded-[28px] p-8 max-w-sm w-full shadow-2xl relative animate-in zoom-in-95 duration-200">
                        <button onClick={() => setShowAuthModal(false)} className="absolute top-4 right-4 w-10 h-10 flex items-center justify-center rounded-full hover:bg-black/5 text-[#49454F]"><span className="material-symbols-rounded text-[24px]">close</span></button>
                        <h2 className="text-[28px] font-seymour text-[#1D192B] mb-6">{authMode === 'login' ? 'Вход' : 'Регистрация'}</h2>
                        {authError && <div className="mb-4 text-sm text-[#FF2B4A] bg-[#FF2B4A]/10 p-3 rounded-lg font-bold">{authError}</div>}

                        <form onSubmit={handleAuthSubmit} className="flex flex-col gap-4">
                            {authMode === 'register' && (
                                <>
                                    <div className="flex flex-col"><label className="text-xs font-semibold text-[#49454F] mb-1 px-1">Никнейм</label><input type="text" required value={authForm.nickname} onChange={(e) => setAuthForm({...authForm, nickname: e.target.value})} className="bg-[#f3f4f6] rounded-t-lg border-b-2 border-[#CAC4D0] focus:border-[#F6828C] px-4 py-3 outline-none" placeholder="Ваше имя" /></div>
                                    <div className="flex flex-col"><label className="text-xs font-semibold text-[#49454F] mb-1 px-1">Телефон</label><input type="tel" required value={authForm.phone} onChange={(e) => setAuthForm({...authForm, phone: e.target.value})} className="bg-[#f3f4f6] rounded-t-lg border-b-2 border-[#CAC4D0] focus:border-[#F6828C] px-4 py-3 outline-none" placeholder="+7 (999) 000-00-00" /></div>
                                </>
                            )}
                            <div className="flex flex-col"><label className="text-xs font-semibold text-[#49454F] mb-1 px-1">Логин (username)</label><input type="text" required value={authForm.username} onChange={(e) => setAuthForm({...authForm, username: e.target.value})} className="bg-[#f3f4f6] rounded-t-lg border-b-2 border-[#CAC4D0] focus:border-[#F6828C] px-4 py-3 outline-none" placeholder="user123" /></div>
                            <div className="flex flex-col"><label className="text-xs font-semibold text-[#49454F] mb-1 px-1">Пароль</label><input type="password" required value={authForm.password} onChange={(e) => setAuthForm({...authForm, password: e.target.value})} className="bg-[#f3f4f6] rounded-t-lg border-b-2 border-[#CAC4D0] focus:border-[#F6828C] px-4 py-3 outline-none" placeholder="••••••••" /></div>
                            <div className="mt-4 flex items-center justify-between">
                                <button type="button" onClick={() => {setAuthMode(authMode === 'login' ? 'register' : 'login'); setAuthError('');}} className="text-[#F6828C] text-sm font-semibold hover:underline px-2">{authMode === 'login' ? 'Создать аккаунт' : 'Уже есть аккаунт?'}</button>
                                <button type="submit" className="bg-[#F6828C] text-white font-bold py-2.5 px-6 rounded-full hover:shadow-lg transition-all active:scale-[0.98]">{authMode === 'login' ? 'Войти' : 'Регистрация'}</button>
                            </div>
                        </form>
                    </div>
                </div>
            )}
        </div>
    );
}