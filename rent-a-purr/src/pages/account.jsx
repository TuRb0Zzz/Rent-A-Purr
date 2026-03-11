import React, { useState, useEffect } from 'react';
import { Calendar, Clock, PawPrint, ShieldCheck, User } from 'lucide-react';
import { api } from '../api';

export default function Account() {
    const [profile, setProfile] = useState(null);
    const [cats, setCats] = useState([]);
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        Promise.all([api.getProfile(), api.getCats()])
            .then(([profRes, catsRes]) => {
                setProfile(profRes.user);
                setCats(catsRes.cats);
                setLoading(false);
            })
            .catch(e => {
                setLoading(false);
            });
    },[]);

    if (loading) return <div className="p-8 text-center font-bold text-slate-400">Загрузка профиля...</div>;
    if (!profile) return <div className="p-8 text-center font-bold text-[#FF2B4A]">Не удалось загрузить профиль. Вы авторизованы?</div>;

    return (
        <div className="min-h-full bg-white text-slate-900 font-m3 pb-12 relative overflow-hidden">
            <div className="w-full px-4 sm:px-6 lg:px-8 pt-6 pb-2 md:pt-8 max-w-[1600px] mx-auto">
                <div className="flex w-full h-20 sm:h-24 md:h-32 lg:h-40 gap-2 sm:gap-3 md:gap-5">
                    <div className="w-5 sm:w-8 md:w-10 lg:w-14 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                    <div className="flex-1 bg-[#F6828C] rounded-full flex items-center justify-center px-4 sm:px-8 shadow-sm">
                        <h1 className="font-seymour text-2xl sm:text-4xl md:text-5xl lg:text-[4.5rem] text-white tracking-wider">Кабинетик</h1>
                    </div>
                    <div className="w-12 sm:w-20 md:w-28 lg:w-40 bg-[#F6828C] rounded-full shrink-0 shadow-sm"></div>
                </div>
            </div>

            <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8 relative z-10">
                <div className="grid grid-cols-1 lg:grid-cols-12 gap-8">

                    <div className="lg:col-span-4 flex flex-col gap-6">
                        <div className="bg-white rounded-[32px] p-6 shadow-sm border-2 border-[#F6828C]/10 relative overflow-hidden">
                            <div className="flex flex-col items-center text-center relative z-10 pt-4">
                                <div className="w-24 h-24 bg-slate-100 rounded-full flex items-center justify-center mb-4"><User size={40} className="text-slate-400"/></div>
                                <h2 className="text-3xl font-extrabold text-slate-800 mb-1">{profile.nickname}</h2>
                                <p className="text-slate-500 mb-8 flex items-center gap-1.5 font-medium"><ShieldCheck size={18} className="text-[#00D26A]" /> ID: {profile.user_id}</p>
                                <div className="w-full bg-slate-50 p-4 rounded-2xl border border-slate-100 text-left">
                                    <p className="text-xs text-slate-400 font-bold uppercase">Email</p>
                                    <p className="text-slate-700 font-bold truncate">{profile.email || "Нет email"}</p>
                                </div>
                            </div>
                        </div>
                    </div>

                    <div className="lg:col-span-8">
                        <h2 className="text-3xl font-extrabold mb-6 text-slate-800 px-2 tracking-tight">Мои аренды</h2>
                        {profile.bookings.length === 0 ? (
                            <div className="bg-slate-50 rounded-[32px] p-12 text-center border-2 border-dashed border-slate-200">
                                <PawPrint size={64} className="text-[#F6828C] mb-6 mx-auto" />
                                <h3 className="text-2xl font-extrabold text-slate-800 mb-2">У вас пока нет активных аренд</h3>
                            </div>
                        ) : (
                            <div className="space-y-6">
                                {profile.bookings.map((booking, idx) => {
                                    const cat = cats.find(c => c.id === booking.cat_id) || {};
                                    return (
                                        <div key={idx} className="bg-white rounded-[32px] p-4 sm:p-6 shadow-sm border-2 border-slate-100 flex flex-col sm:flex-row gap-6">
                                            <div className="relative w-full sm:w-48 h-48 flex-shrink-0">
                                                <img src={cat.filename?.[0]} alt="cat" className="w-full h-full object-cover rounded-[24px]"/>
                                            </div>
                                            <div className="flex-1 flex flex-col justify-center">
                                                <div className="flex justify-between items-start gap-2 mb-2">
                                                    <div><h3 className="text-2xl font-extrabold">{cat.name || "Кот удален"}</h3><p className="text-slate-500 font-bold">{cat.breed}</p></div>
                                                    <span className={booking.status === 1 ? "bg-[#00D26A]/10 text-[#00A855] px-4 py-1.5 rounded-full text-xs font-extrabold uppercase" : "bg-[#FFB300]/10 text-[#B37E00] px-4 py-1.5 rounded-full text-xs font-extrabold uppercase"}>
                                {booking.status === 1 ? "Подтверждено" : "Ожидает"}
                            </span>
                                                </div>
                                                <div className="mt-4 bg-slate-50 px-4 py-3 rounded-2xl text-sm">
                                                    <span className="flex items-center gap-2 text-slate-500 font-bold"><Calendar size={16} className="text-[#7838F5]"/> {booking.start_time.substring(0, 10)}</span>
                                                    <span className="flex items-center gap-2 text-slate-800 font-extrabold mt-1"><Clock size={16} className="text-[#FF6B00]"/> {booking.start_time.substring(11, 16)} — {booking.end_time.substring(11, 16)}</span>
                                                </div>
                                            </div>
                                        </div>
                                    )
                                })}
                            </div>
                        )}
                    </div>
                </div>
            </main>
        </div>
    );
}