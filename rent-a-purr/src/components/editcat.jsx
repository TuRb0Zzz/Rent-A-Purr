import React, { useState } from 'react';
import { createPortal } from 'react-dom';
import { X } from 'lucide-react';

export default function EditCat({ cat, onClose, onSubmit }) {
    const [formData, setFormData] = useState({
        name: cat.name, breed: cat.breed, age: cat.age, image: cat.image,
        description: cat.description, tags: cat.tags.join(', ')
    });

    const handleSubmit = (e) => {
        e.preventDefault();
        onSubmit({
            ...cat, ...formData,
            tags: formData.tags.split(',').map(tag => tag.trim()).filter(Boolean),
        });
    };

    return createPortal(
        <div className="fixed inset-0 z-[9999] flex items-center justify-center p-4 bg-slate-900/40 backdrop-blur-sm animate-in fade-in duration-200 font-m3">
            <div className="bg-white rounded-[32px] w-full max-w-2xl shadow-2xl border-2 border-[#F6828C]/20 overflow-hidden flex flex-col max-h-[90vh]">
                <div className="px-8 py-6 border-b-2 border-slate-100 flex justify-between items-center bg-white">
                    <h2 className="text-3xl font-extrabold text-slate-800 tracking-tight">Редактировать кота</h2>
                    <button onClick={onClose} className="p-2 text-slate-400 hover:text-slate-800 hover:bg-slate-100 rounded-full"><X size={28} strokeWidth={2.5} /></button>
                </div>
                <div className="p-8 overflow-y-auto no-scrollbar">
                    <form id="edit-cat-form" onSubmit={handleSubmit} className="space-y-6">
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Имя кота *</label><input required value={formData.name} onChange={e => setFormData({...formData, name: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" /></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Порода *</label><input required value={formData.breed} onChange={e => setFormData({...formData, breed: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" /></div>
                        </div>
                        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Возраст *</label><input required value={formData.age} onChange={e => setFormData({...formData, age: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" /></div>
                            <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Фотография (URL)</label><input value={formData.image} onChange={e => setFormData({...formData, image: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" /></div>
                        </div>
                        <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Описание *</label><textarea required value={formData.description} onChange={e => setFormData({...formData, description: e.target.value})} rows="3" className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold resize-none"></textarea></div>
                        <div className="space-y-2"><label className="text-sm font-extrabold text-slate-700 ml-1 uppercase">Характеристики (через запятую)</label><input value={formData.tags} onChange={e => setFormData({...formData, tags: e.target.value})} className="w-full px-5 py-4 rounded-2xl bg-slate-50 border-2 border-slate-100 focus:border-[#F6828C] outline-none font-bold" /></div>
                    </form>
                </div>
                <div className="px-8 py-5 border-t-2 border-slate-100 bg-white flex justify-end gap-3 rounded-b-[32px]">
                    <button onClick={onClose} className="px-6 py-3.5 rounded-2xl text-slate-500 font-extrabold hover:bg-slate-100 transition-colors">Отмена</button>
                    <button form="edit-cat-form" type="submit" className="px-8 py-3.5 rounded-2xl bg-[#7838F5] hover:bg-[#6025D1] text-white font-extrabold shadow-lg shadow-[#7838F5]/30 text-lg">Сохранить</button>
                </div>
            </div>
        </div>,
        document.body
    );
}